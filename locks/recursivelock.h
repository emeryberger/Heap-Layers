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

#ifndef HL_RECURSIVELOCK_H
#define HL_RECURSIVELOCK_H

#include <cassert>
#include "threads/cpuinfo.h"

/**
 * @class RecursiveLockType
 * @brief Implements a recursive lock using some base lock representation.
 * @param BaseLock The base lock representation.
 */

namespace HL {

  template <class BaseLock>
  class RecursiveLockType : public BaseLock {
  public:

    inline RecursiveLockType (void)
      : _tid (-1),
	_recursiveDepth (0)
    {}

    inline void lock() {
      auto currthread = CPUInfo::getThreadId();
      if (_tid == currthread) {
	_recursiveDepth++;
      } else {
	BaseLock::lock();
	_tid = currthread;
	_recursiveDepth++;
      }
    }

    inline void unlock (void) {
      auto currthread = (int) CPUInfo::getThreadId();
      if (_tid == currthread) {
	_recursiveDepth--;
	if (_recursiveDepth == 0) {
	  _tid = -1;
	  BaseLock::unlock();
	}
      } else {
	// We tried to unlock it but we didn't lock it!
	// This should never happen.
	assert (0);
	abort();
      }
    }

  private:
    int _tid;	                /// The lock owner's thread id. -1 if unlocked.
    int _recursiveDepth;	/// The recursion depth of the lock.
  };

}




#endif
