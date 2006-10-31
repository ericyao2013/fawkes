
/***************************************************************************
 *  refc_rwlock.h - Read Write Lock with reference counter
 *
 *  Generated: Fri Oct 27 17:25:44 2006
 *  Copyright  2006  Tim Niemueller [www.niemueller.de]
 *
 *  $Id$
 *
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software Foundation,
 *  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02111-1307, USA.
 */

#ifndef __CORE_THREADING_REF_RWLOCK_H_
#define __CORE_THREADING_REF_RWLOCK_H_

#include <core/threading/read_write_lock.h>
#include <core/utils/refcount.h>

class RefCountRWLock : public ReadWriteLock, public RefCount
{
 public:
  RefCountRWLock(ReadWriteLock::ReadWriteLockPolicy policy = ReadWriteLock::RWLockPolicyPreferWriter);

  virtual ~RefCountRWLock();

};


#endif
