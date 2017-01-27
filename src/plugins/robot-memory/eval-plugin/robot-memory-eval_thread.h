
/***************************************************************************
 *  robot-memory-eval_thread.h - robot-memory-eval
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

#ifndef __PLUGINS_ROBOT_MEMORY_EVAL_THREAD_H_
#define __PLUGINS_ROBOT_MEMORY_EVAL_THREAD_H_

#include <core/threading/thread.h>
#include <aspect/blocked_timing.h>
#include <aspect/logging.h>
#include <aspect/blackboard.h>
#include <aspect/configurable.h>
#include <plugins/robot-memory/aspect/robot_memory_aspect.h>
#include <plugins/mongodb/aspect/mongodb.h>
#include <random>

#define NUMBER_STEPS 8
#define GENERATION 0
#define BENCHMARK_INSERT 1
#define BENCHMARK_QUERY_NOT_FOUND 2
#define BENCHMARK_QUERY_FOUND 3
#define BENCHMARK_QUERY_COMPARE 4
#define BENCHMARK_QUERY_COMPARE_ALL 5
#define BENCHMARK_UPDATE 6
#define BENCHMARK_REMOVE 7

namespace fawkes {
  // add forward declarations here, e.g., interfaces
}

class RobotMemoryEvalThread 
: public fawkes::Thread,
  public fawkes::BlockedTimingAspect,
  public fawkes::LoggingAspect,
  public fawkes::ConfigurableAspect,
  public fawkes::BlackBoardAspect,
  public fawkes::RobotMemoryAspect,
  public fawkes::MongoDBAspect
{

 public:
  RobotMemoryEvalThread();

  virtual void init();
  virtual void finalize();
  virtual void loop();

  /** Stub to see name in backtrace for easier debugging. @see Thread::run() */
  protected: virtual void run() { Thread::run(); }

 private:
  int gen_ben_step_;
  bool finished_;

  std::string collection_;
  int object_counter_;

  int start_size_;
  int step_size_;
  int final_size_;

  int storage_places_;
  float prob_dislocated_;

  std::mt19937 rng;

  void generate(int amount);
  void benchmark_insert();
  void benchmark_query_not_found();
  void benchmark_query_found();
  void benchmark_query_compare();
  void benchmark_query_compare_all();
  void benchmark_update();
  void benchmark_remove();
  int get_place(int storage);
  int get_random_storage();
};


#endif
