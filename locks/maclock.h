/* -*- C++ -*- */

/*

  Heap Layers: An Extensible Memory Allocation Infrastructure
  
  Copyright (C) 2000-2020 by Emery Berger
  http://www.emeryberger.com
  emery@cs.umass.edu
  
  Heap Layers is distributed under the terms of the Apache 2.0 license.

  You may obtain a copy of the License at
  http://www.apache.org/licenses/LICENSE-2.0

*/


#ifndef HL_MACLOCK_H
#define HL_MACLOCK_H

#if defined(__APPLE__)

/**
 * @class MacLockType
 * @brief Locking using OS X spin locks.
 */

#if defined(__MAC_OS_X_VERSION_MIN_REQUIRED) && __MAC_OS_X_VERSION_MIN_REQUIRED >= 101200
#define USE_UNFAIR_LOCKS 1
#include <os/lock.h>
#else
#define USE_UNFAIR_LOCKS 0
#include <libkern/OSAtomic.h>
#endif


namespace HL {

  class MacLockType {
  public:

    MacLockType()
    {
#if USE_UNFAIR_LOCKS
      mutex = OS_UNFAIR_LOCK_INIT;
#else
      mutex = OS_SPINLOCK_INIT;
#endif      
    }

    ~MacLockType()
    {
#if USE_UNFAIR_LOCKS
      mutex = OS_UNFAIR_LOCK_INIT;
#else
      mutex = OS_SPINLOCK_INIT;
#endif      
    }

    inline void lock() {
#if USE_UNFAIR_LOCKS
      os_unfair_lock_lock(&mutex);
#else
      OSSpinLockLock (&mutex);
#endif
    }

    inline void unlock() {
#if USE_UNFAIR_LOCKS
      os_unfair_lock_unlock(&mutex);
#else
      OSSpinLockUnlock (&mutex);
#endif
    }

  private:

#if USE_UNFAIR_LOCKS
    os_unfair_lock mutex;
#else
    OSSpinLock mutex;
#endif

  };

}

#endif

#endif
