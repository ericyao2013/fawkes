
/***************************************************************************
 *  robot-memory-eval_thread.cpp - robot-memory-eval
 *
 *  Created: Thu Jan 26 15:54:26 2017
 *  Copyright  2017  Frederik Zwilling
 ****************************************************************************/

/*  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  Read the full text in the LICENSE.GPL file in the doc directory.
 */

#include "robot-memory-eval_thread.h"
#include <baseapp/run.h>

using namespace fawkes;
using namespace mongo;

/** @class RobotMemoryEvalThread 'robot-memory-eval_thread.h' 
 * Performs robot-memory operations for evaluation
 * @author Frederik Zwilling
 */

/** Constructor. */
RobotMemoryEvalThread::RobotMemoryEvalThread()
 : Thread("RobotMemoryEvalThread", Thread::OPMODE_WAITFORWAKEUP),
   BlockedTimingAspect(BlockedTimingAspect::WAKEUP_HOOK_WORLDSTATE),
   MongoDBAspect("robotmemory-local")
{
}

void
RobotMemoryEvalThread::init()
{
  finished_ = false;
  gen_ben_step_ = 0;
  object_counter_ = 0;

  std::random_device rd;
  rng = std::mt19937(rd());

  //load config
  collection_ = config->get_string("plugins/robot-memory-eval/collection");
  start_size_ = config->get_int("plugins/robot-memory-eval/start-size");
  step_size_ = config->get_int("plugins/robot-memory-eval/step-size");
  final_size_ = config->get_int("plugins/robot-memory-eval/final-size");
  storage_places_ = config->get_int("plugins/robot-memory-eval/storage-places");
  prob_dislocated_ = config->get_float("plugins/robot-memory-eval/probability-dislocated");

  logger->log_info(name(), "Starting Robot Memory Evaluation");

  mongodb_client->remove(collection_, fromjson("{}"));
  mongodb_client->remove("eval.durations", fromjson("{}"));
}

void
RobotMemoryEvalThread::loop()
{
  if(finished_ && gen_ben_step_ == GENERATION)
  {
    //stop fawkes to finish the testing run
    fawkes::runtime::quit();
    return;
  }

  if(gen_ben_step_ == GENERATION)
    generate(step_size_);
  if(gen_ben_step_ == BENCHMARK_INSERT)
    benchmark_insert();
  if(gen_ben_step_ == BENCHMARK_QUERY_NOT_FOUND)
    benchmark_query_not_found();
  if(gen_ben_step_ == BENCHMARK_QUERY_FOUND)
    benchmark_query_found();
  if(gen_ben_step_ == BENCHMARK_QUERY_COMPARE)
    benchmark_query_compare();
  if(gen_ben_step_ == BENCHMARK_QUERY_COMPARE_ALL)
    benchmark_query_compare_all();
  if(gen_ben_step_ == BENCHMARK_UPDATE)
    benchmark_update();
  if(gen_ben_step_ == BENCHMARK_REMOVE)
    benchmark_remove();

  if(gen_ben_step_ == NUMBER_STEPS - 1 && object_counter_ >= final_size_)
  {
    logger->log_info(name(), "Finished Benchmark :-)");
    finished_ = true;
  }
  gen_ben_step_ = (gen_ben_step_ + 1) % NUMBER_STEPS;
}

void
RobotMemoryEvalThread::finalize()
{
}

void
RobotMemoryEvalThread::generate(int amount)
{
  logger->log_info(name(), "Generating: %d", object_counter_);
  for(int i = 0; i < amount; i++)
  {
    BSONObjBuilder b;
    b << "name" << std::to_string(object_counter_);
    int storage = get_random_storage();
    b << "storage" << std::to_string(storage);
    b << "place" << std::to_string(get_place(storage));
    mongodb_client->insert(collection_, b.obj());
    object_counter_++;
  }
}

void
RobotMemoryEvalThread::benchmark_insert()
{
  //random object:
  int object = std::uniform_int_distribution<int>(0, object_counter_)(rng);

  BSONObjBuilder b;
  b << "db-size" << object_counter_;
  b << "name" << std::to_string(object);
  int storage = get_random_storage();
  b << "storage" << std::to_string(storage);
  b << "place" << std::to_string(get_place(storage));
  object_counter_++;
  robot_memory->insert(b.obj(), collection_);
}

void
RobotMemoryEvalThread::benchmark_query_not_found()
{
  //random non-existing object:
  int object = std::uniform_int_distribution<int>(final_size_, final_size_+object_counter_)(rng);

  BSONObjBuilder b;
  b << "$comment" << BSON("exp" << "not-found" << "db-size" << object_counter_);
  b << "name" << std::to_string(object);
  QResCursor cur = robot_memory->query(b.obj(), collection_);
  cur->more();
}

void
RobotMemoryEvalThread::benchmark_query_found()
{
  //random object:
  int object = std::uniform_int_distribution<int>(0, object_counter_)(rng);

  BSONObjBuilder b;
  b << "$comment" << BSON("exp" << "found" << "db-size" << object_counter_);
  b << "name" << std::to_string(object);
  QResCursor cur = robot_memory->query(b.obj(), collection_);
  cur->more();
}

void
RobotMemoryEvalThread::benchmark_query_compare()
{
  BSONObjBuilder b;
  b << "$comment" << BSON("exp" << "compare" << "db-size" << object_counter_);
  b << "$where" << "obj.storage != obj.place";
  QResCursor cur = robot_memory->query(b.obj(), collection_);
  cur->more();
}

void
RobotMemoryEvalThread::benchmark_query_compare_all()
{
  BSONObjBuilder b;
  b << "$comment" << BSON("exp" << "compare-all" << "db-size" << object_counter_);
  b << "$where" << "obj.storage == 'gibts nicht'";
  QResCursor cur = robot_memory->query(b.obj(), collection_);
  cur->more();
}

void
RobotMemoryEvalThread::benchmark_update()
{
  //random object:
  int object = std::uniform_int_distribution<int>(0, object_counter_)(rng);

  BSONObjBuilder b;
  b << "$comment" << BSON("exp" << "update" << "db-size" << object_counter_);
  b << "name" << std::to_string(object);
  robot_memory->update(b.obj(), BSON("name" << std::to_string(object) << "updated" << 1), collection_);
}

void
RobotMemoryEvalThread::benchmark_remove()
{
  //random object:
  int object = std::uniform_int_distribution<int>(0, object_counter_)(rng);

  BSONObjBuilder b;
  b << "$comment" << BSON("exp" << "remove" << "db-size" << object_counter_);
  b << "name" << std::to_string(object);
  robot_memory->remove(b.obj(), collection_);
}

int
RobotMemoryEvalThread::get_place(int storage)
{
  if(((double) rand() / (double)RAND_MAX)  < prob_dislocated_)
    return get_random_storage();
  else
    return storage;
}

int
RobotMemoryEvalThread::get_random_storage()
{
  return std::uniform_int_distribution<int>(0, storage_places_)(rng);
}

