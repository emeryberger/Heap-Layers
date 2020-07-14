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

#ifndef HL_WINLOCK_H
#define HL_WINLOCK_H

#if defined(_WIN32)

#include <windows.h>
#include <winnt.h>

/**
 * @class WinLockType
 * @brief Locking using Win32 mutexes.
 *
 * Note that this lock type detects whether we are running on a
 * multiprocessor.  If not, then we do not use atomic operations.
 */

namespace HL {

  class WinLockType {
  public:

    WinLockType (void)
      : mutex (0)
    {}

    ~WinLockType (void)
    {
      mutex = 0;
    }

    inline void lock (void) {
      int spinCount = 0;
      while (InterlockedExchange ((long *) &mutex, 1) != 0) {
	while (mutex != 0) {
	  YieldProcessor();
	}
      }
    }

    inline void unlock (void) {
      mutex = 0;
      // InterlockedExchange (&mutex, 0);
    }

  private:
    unsigned int mutex;
    bool onMultiprocessor (void) {
      SYSTEM_INFO infoReturn[1];
      GetSystemInfo (infoReturn);
      if (infoReturn->dwNumberOfProcessors == 1) {
	return FALSE;
      } else {
	return TRUE;
      }
    }
  };

}

#endif

#endif
