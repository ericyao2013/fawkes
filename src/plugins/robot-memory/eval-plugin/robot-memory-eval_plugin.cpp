
/***************************************************************************
 *  robot-memory-eval_plugin.cpp - robot-memory-eval
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

#include <core/plugin.h>

#include "robot-memory-eval_thread.h"

using namespace fawkes;

/** @class RobotMemoryEvalPlugin "robot-memory-eval_plugin.cpp"
 * Performs robot-memory operations for evaluation
 * @author Frederik Zwilling
 */
class RobotMemoryEvalPlugin : public fawkes::Plugin
{
 public:
  /** Constructor.
   * @param config Fakwes configuration
   */
  RobotMemoryEvalPlugin(Configuration *config)
     : Plugin(config)
  {
     thread_list.push_back(new RobotMemoryEvalThread());
  }
};

PLUGIN_DESCRIPTION("Performs robot-memory operations for evaluation")
EXPORT_PLUGIN(RobotMemoryEvalPlugin)