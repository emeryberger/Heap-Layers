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

#ifndef HL_POSIXLOCK_H
#define HL_POSIXLOCK_H

#if !defined(_WIN32)

#include <pthread.h>

/**
 * @class PosixLockType
 * @brief Locking using POSIX mutex objects.
 */

namespace HL {

  class PosixLockType {
  public:

    PosixLockType (void)
    {
      int r = pthread_mutex_init (&mutex, NULL);
      if (r) {
	throw 0;
      }
    }
  
    ~PosixLockType (void)
    {
      pthread_mutex_destroy (&mutex);
    }
  
    void lock (void) {
      pthread_mutex_lock (&mutex);
    }
  
    void unlock (void) {
      pthread_mutex_unlock (&mutex);
    }
  
  private:
    union {
      pthread_mutex_t mutex;
      double _dummy[sizeof(pthread_mutex_t)/sizeof(double) + 1];
    };
  };

  typedef PosixLockType PosixLock;
  
}

#endif

#endif
