
/***************************************************************************
 *  gazsim_pantilt_thread.h - Plugin used to simulate a pantilt
 *
 *  Created: Thu Feb 25 09:22:04 2016
 *  Copyright  2016 Frederik Zwilling
 *
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

#include "gazsim_pantilt_thread.h"

#include <utils/math/angle.h>
#include <core/threading/mutex_locker.h>
#include <core/threading/read_write_lock.h>
#include <core/threading/wait_condition.h>
#include <boost/lexical_cast.hpp>
#include <config/change_handler.h>

#include <cstdarg>
#include <cmath>
#include <unistd.h>

using namespace fawkes;
using namespace gazebo;

/** @class GazsimPantiltThread "gazsim_pantilt_thread.h"
 * Thread simulates the Pantilt in Gazebo
 *
 * @author Frederik Zwilling
 */

/** Constructor. */

GazsimPantiltThread::GazsimPantiltThread()
  : Thread("GazsimPantiltThread", Thread::OPMODE_WAITFORWAKEUP),
    BlockedTimingAspect(BlockedTimingAspect::WAKEUP_HOOK_ACT_EXEC)
{
  set_name("GazsimPantiltThread()");
}


void
GazsimPantiltThread::init()
{
  logger->log_debug(name(), "Initializing Simulation of the PanTilt Plugin");
  
  pantilt_if_name_ = config->get_string("/gazsim/pantilt/if-name");
  cfg_prefix_ = config->get_string("/gazsim/pantilt/cfg-prefix");

  //setup pantilt if with default values
  pantilt_if_ = blackboard->open_for_writing<PanTiltInterface>(pantilt_if_name_.c_str());
  pantilt_if_->set_calibrated(true);
  pantilt_if_->set_min_pan(config->get_float(cfg_prefix_ + "pan_min"));
  pantilt_if_->set_max_pan(config->get_float(cfg_prefix_ + "pan_max"));
  pantilt_if_->set_min_tilt(config->get_float(cfg_prefix_ + "tilt_min"));
  pantilt_if_->set_max_tilt(config->get_float(cfg_prefix_ + "tilt_max"));
  pantilt_if_->set_pan_margin(config->get_float(cfg_prefix_ + "pan_margin"));
  pantilt_if_->set_tilt_margin(config->get_float(cfg_prefix_ + "tilt_margin"));
  pantilt_if_->set_max_pan_velocity(0);
  pantilt_if_->set_max_tilt_velocity(0);
  pantilt_if_->set_pan_velocity(0);
  pantilt_if_->set_tilt_velocity(0);
  pantilt_if_->write();
}

void
GazsimPantiltThread::finalize()
{
  blackboard->close(pantilt_if_);
}

void
GazsimPantiltThread::loop()
{
  // pantilt_if_->set_final(__servo_if_left->is_final() && __servo_if_right->is_final());

  // process interface messages
  while (! pantilt_if_->msgq_empty() ) {
    if (pantilt_if_->msgq_first_is<PanTiltInterface::StopMessage>()) {
      logger->log_warn(name(), "%s is not implemented in the simulation.",
                       pantilt_if_->msgq_first()->type());
    } else if (pantilt_if_->msgq_first_is<PanTiltInterface::FlushMessage>()) {
      logger->log_warn(name(), "%s is not implemented in the simulation.",
                       pantilt_if_->msgq_first()->type());
    } else if (pantilt_if_->msgq_first_is<PanTiltInterface::CalibrateMessage>()) {
      logger->log_warn(name(), "%s is not implemented in the simulation.",
                       pantilt_if_->msgq_first()->type());
    } else if (pantilt_if_->msgq_first_is<PanTiltInterface::ParkMessage>()) {
      logger->log_warn(name(), "%s is not implemented in the simulation.",
                       pantilt_if_->msgq_first()->type());
    } else if (pantilt_if_->msgq_first_is<PanTiltInterface::GotoMessage>()) {
      logger->log_warn(name(), "%s is not implemented in the simulation.",
                       pantilt_if_->msgq_first()->type());
      PanTiltInterface::GotoMessage *msg = pantilt_if_->msgq_first(msg);
      //send_pantilt_msg();
      pantilt_if_->set_msgid(msg->id());
      pantilt_if_->set_final(true);
      pantilt_if_->write();
    } else if (pantilt_if_->msgq_first_is<PanTiltInterface::TimedGotoMessage>()) {
      logger->log_warn(name(), "%s is not implemented in the simulation.",
                       pantilt_if_->msgq_first()->type());
    } else if (pantilt_if_->msgq_first_is<PanTiltInterface::SetEnabledMessage>()) {
      logger->log_warn(name(), "%s is not implemented in the simulation.",
                       pantilt_if_->msgq_first()->type());
    } else if (pantilt_if_->msgq_first_is<PanTiltInterface::SetVelocityMessage>()) {
      logger->log_warn(name(), "%s is not implemented in the simulation.",
                       pantilt_if_->msgq_first()->type());
    } else if (pantilt_if_->msgq_first_is<PanTiltInterface::SetMarginMessage>()) {
      logger->log_warn(name(), "%s is not implemented in the simulation.",
                       pantilt_if_->msgq_first()->type());
    } else {
      logger->log_warn(name(), "Unknown message received");
    }

    pantilt_if_->msgq_pop();
    pantilt_if_->write();
  }
}

void GazsimPantiltThread::send_pantilt_msg(int value)
{
  // TODO: send message to gazebo
}
