/***************************************************************************
 *  computables_manager.cpp - Class managing registered computables and
 *                   checking if any computables are invoced by a query
 *
 *  Created: 6:37:45 PM 2016
 *  Copyright  2016  Frederik Zwilling
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

#include "computables_manager.h"
#include <core/exception.h>
#include <plugins/robot-memory/robot_memory.h>
#include <chrono>

/** @class ComputablesManager  computables_manager.h
 *  This class manages registering computables and can check
 *  if any computables are invoced by a query.
 * @author Frederik Zwilling
 */

using namespace fawkes;
using namespace mongo;

ComputablesManager::ComputablesManager(fawkes::Logger* logger, fawkes::Configuration* config,
  RobotMemory* robot_memory, fawkes::Clock* clock)
{
  logger_ = logger;
  config_ = config;
  robot_memory_ = robot_memory;
  clock_ = clock;

  matching_test_collection_ = "robmem.computables_matching";
  try {
    matching_test_collection_ = config_->get_string("/plugins/robot-memory/database") + ".computables_matching";
  } catch (Exception &e) {}

  experiment_name_ = config_->get_string("/plugins/robot-memory/eval/experiment");

  srand(time(NULL));
}

ComputablesManager::~ComputablesManager()
{
}


/**
 * Remove previously registered computable
 * @param computable The computable to remove
 */
void ComputablesManager::remove_computable(Computable* computable)
{
  for(std::list<Computable*>::iterator it = computables.begin(); it != computables.end(); it++)
  {
    if((*it) == computable)
    {
      Computable* comp = *it;
      computables.erase(it);
      delete comp;
      return;
    }
  }
}


/**
 * Checks if computable knowledge is queried and calls the compute functions in this case
 * @param query The query that might ask for computable knowledge
 * @param collection The collection that is querried
 * @return Were computed documents added?
 */
bool ComputablesManager::check_and_compute(mongo::Query query, std::string collection)
{
  // measure time for evaluation
  BSONObjBuilder b_time;
  b_time << "op" << "computable";
  b_time << "query" << query.obj.copy();
  b_time << "experiment" << experiment_name_;
  BSONArrayBuilder arr_computables;
  std::chrono::time_point<std::chrono::system_clock> start, end, prepare_end, comp_start, comp_end, check_start, check_end, insert_start, insert_end;
  start = std::chrono::system_clock::now();

  //check if computation result of the query is already cached
  for(std::map<std::tuple<std::string, std::string>, long long>::iterator it = cached_querries_.begin();
      it != cached_querries_.end(); it++)
  {
    if(collection == std::get<0>(it->first) && query.toString() == std::get<1>(it->first))
    {
      return false;
    }
  }
  if(collection.find(matching_test_collection_) != std::string::npos)
    return false; //not necessary for matching test itself
  bool added_computed_docs = false;
  //check if the query is matched by the computable identifyer
  //to do that we just insert the query as if it would be a document and query for it with the computable identifiers
  std::string current_test_collection = matching_test_collection_ + std::to_string(rand());
  robot_memory_->insert(query.obj, current_test_collection);
  prepare_end = std::chrono::system_clock::now();
  for(std::list<Computable*>::iterator it = computables.begin(); it != computables.end(); it++)
  {
    BSONObjBuilder b_time_comp;
    b_time_comp << "computable" << (*it)->get_query().obj.copy();
    check_start = std::chrono::system_clock::now();
    if(collection == (*it)->get_collection() &&  robot_memory_->query((*it)->get_query(), current_test_collection)->more())
    {
      check_end = std::chrono::system_clock::now();
      std::chrono::duration<double> elapsed_check = check_end - check_start;
      b_time_comp << "check" << elapsed_check.count();
      comp_start = std::chrono::system_clock::now();
      std::list<BSONObj> computed_docs_list = (*it)->compute(query.obj);
      comp_end = std::chrono::system_clock::now();
      std::chrono::duration<double> elapsed_compute = comp_end - comp_start;
      b_time_comp << "compute" << elapsed_compute.count();
      if(computed_docs_list.size() > 0)
      {
        insert_start = std::chrono::system_clock::now();
        //move list into vector
        std::vector<BSONObj> computed_docs_vector{ std::make_move_iterator(std::begin(computed_docs_list)),
          std::make_move_iterator(std::end(computed_docs_list))};
        //remember how long a query is cached:
        long long cached_until = computed_docs_vector[0].getField("_robmem_info").Obj().getField("cached_until").Long();
        cached_querries_[std::make_tuple(collection, query.toString())] = cached_until;
        //TODO: fix problem: equivalent queries in different order jield unequal strings
        robot_memory_->insert(computed_docs_vector, (*it)->get_collection());
        added_computed_docs = true;
        insert_end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_insert = insert_end - insert_start;
        b_time_comp << "insert" << elapsed_insert.count();
      }
    }
    else
    {
      check_end = std::chrono::system_clock::now();
      std::chrono::duration<double> elapsed_check = check_end - check_start;
      b_time_comp << "check" << elapsed_check.count();
    }
    arr_computables.append(b_time_comp.obj());
  }
  robot_memory_->drop_collection(current_test_collection);

  //measure time and log
  end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_total = end - start;
  b_time << "total" << elapsed_total.count();
  std::chrono::duration<double> elapsed_prepare = prepare_end - start;
  b_time << "prepare" << elapsed_prepare.count();
  b_time << "computables" << arr_computables.arr();
  robot_memory_->insert(b_time.obj(), "eval.computables");

  return added_computed_docs;
}

/**
 * Clean up all collections containing documents computed on demand
 */
void ComputablesManager::cleanup_computed_docs()
{
  long long current_time_ms =
          std::chrono::system_clock::now().time_since_epoch() /
          std::chrono::milliseconds(1);
  for(std::map<std::tuple<std::string, std::string>, long long>::iterator it = cached_querries_.begin();
        it != cached_querries_.end(); it++)
    {
      if(current_time_ms > it->second)
      {
        robot_memory_->remove(BSON("_robmem_info.computed" << true
            << "_robmem_info.cached_until" << BSON("$lt" << current_time_ms)), std::get<0>(it->first));
        cached_querries_.erase(it->first);
      }
    }
}
