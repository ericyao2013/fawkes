/***************************************************************************
 *  syncpoint.h - Fawkes SyncPoint
 *
 *  Created: Thu Jan 09 12:22:03 2014
 *  Copyright  2014  Till Hofmann
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

#ifndef __SYNCPOINT_SYNCPOINT_H_
#define __SYNCPOINT_SYNCPOINT_H_

#include <interface/interface.h>
#include <syncpoint/syncpoint_call.h>
#include <core/threading/mutex.h>
#include <core/threading/wait_condition.h>
#include <utils/time/time.h>

#include <core/utils/refptr.h>
#include <core/utils/circular_buffer.h>

#include <set>
#include <map>

namespace fawkes {
#if 0 /* just to make Emacs auto-indent happy */
}
#endif

class SyncPointManager;


class SyncPoint
{
  public:
    /** Type to define when a thread wakes up after waiting for a SyncPoint.
     * A thread can be either wake up if ANY other thread emits the SyncPoint,
     * or if ALL registered threads emit the SyncPoint.
     */
    typedef enum {
      WAIT_FOR_ONE,
      WAIT_FOR_ALL,
      NONE
    } WakeupType;

    SyncPoint(std::string identifier);
    virtual ~SyncPoint();

    /** send a signal to all waiting threads */
    virtual void emit(const std::string & component);

    /** wait for the sync point to be emitted by any other component */
    virtual void wait(const std::string & component, WakeupType = WAIT_FOR_ONE);
    virtual void wait_for_one(const std::string & component);
    virtual void wait_for_all(const std::string & component);

    /** register as emitter */
    virtual void register_emitter(const std::string & component);

    /** unregister as emitter */
    virtual void unregister_emitter(const std::string & component);

    std::string get_identifier() const;
    bool operator==(const SyncPoint & other) const;
    bool operator<(const SyncPoint & other) const;

    std::set<std::string> get_watchers() const;
    CircularBuffer<SyncPointCall> get_wait_calls(WakeupType type = WAIT_FOR_ONE) const;
    CircularBuffer<SyncPointCall> get_emit_calls() const;


    /**
     * allow Syncpoint Manager to edit
     */
    friend class SyncPointManager;

  protected:
    std::pair<std::set<std::string>::iterator,bool> add_watcher(std::string watcher);

  protected:
    /** The unique identifier of the SyncPoint */
    const std::string identifier_;
    /** Set of all components which use this SyncPoint */
    std::set<std::string> watchers_;
    /** Set of all components which are currently waiting for a single emitter */
    std::set<std::string> watchers_wait_for_one_;
    /** Set of all components which are currently waiting on the barrier */
    std::set<std::string> watchers_wait_for_all_;

    /** A buffer of the most recent emit calls. */
    CircularBuffer<SyncPointCall> emit_calls_;
    /** A buffer of the most recent wait calls of type WAIT_FOR_ONE. */
    CircularBuffer<SyncPointCall> wait_for_one_calls_;
    /** A buffer of the most recent wait calls of type WAIT_FOR_ALL. */
    CircularBuffer<SyncPointCall> wait_for_all_calls_;
    /** Time when this SyncPoint was created */
    const Time creation_time_;

    /** Mutex used to protect all member variables */
    Mutex *mutex_;
    /** WaitCondition which is used for wait_for_one() */
    WaitCondition *cond_wait_for_one_;
    /** WaitCondition which is used for wait_for_all() */
    WaitCondition *cond_wait_for_all_;

  private:
    void reset_emitters();

  private:
    /** The predecessor SyncPoint, which is the SyncPoint one level up
     *  e.g. "/test/sp" -> "/test"
     */
    RefPtr<SyncPoint> predecessor_;

    std::set<std::string> emitters_;
    std::set<std::string> pending_emitters_;
};

} // end namespace fawkes

#endif