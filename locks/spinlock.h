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

#ifndef HL_SPINLOCK_H
#define HL_SPINLOCK_H

#if !defined(_WIN32) && ((__cplusplus < 201103) || defined(__SUNPRO_CC)) // Still no support for atomic...
#include "spinlock-old.h"
#else

#include <atomic> // C++11

#if defined(unix)
#include <sched.h>
#endif

#if defined(__SVR4)
#include <thread.h>
#endif

#include "threads/cpuinfo.h"

#if defined(_MSC_VER)

#if !defined(NO_INLINE)
#pragma inline_depth(255)
#define NO_INLINE __declspec(noinline)
#define INLINE __forceinline
#define inline __forceinline
#endif // !defined(NO_INLINE)

#else

#endif // defined(_MSC_VER)


#if defined(_WIN32) && !defined(_WIN64)

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif

#endif

#define _MM_PAUSE

// The use of the PAUSE instruction has now been disabled, as it can be insanely costly.
// See https://aloiskraus.wordpress.com/2018/06/16/why-skylakex-cpus-are-sometimes-50-slower-how-intel-has-broken-existing-code/amp

/*

#if defined(_WIN32) // includes WIN64

// NOTE: Below is the new "pause" instruction, which is inocuous for
// previous architectures, but crucial for Intel chips with
// hyperthreading.  See
// http://www.usenix.org/events/wiess02/tech/full_papers/nakajima/nakajima.pdf
// for discussion.

#define _MM_PAUSE YieldProcessor() // {__asm{_emit 0xf3};__asm {_emit 0x90}}
#include <windows.h>

#elif defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))

#define _MM_PAUSE  { asm (".byte 0xf3; .byte 0x90" : : : "memory"); }

#else

#define _MM_PAUSE

#endif

*/

namespace HL {

  class SpinLockType {
  private:
    std::atomic<bool> _mutex;
  public:
  
    SpinLockType()
      : _mutex (false)
    {}
  
    ~SpinLockType()
    {}

    inline void lock() {
      if (_mutex.exchange(true)) {
	contendedLock();
      }
    }

    inline bool didLock() {
      return !_mutex.exchange(true);
    }

    inline void unlock() {
      _mutex = false;
    }

  private:

    NO_INLINE
    void contendedLock() {
      const int MAX_SPIN = 1000;
      while (true) {
	if (!_mutex.exchange(true)) {
	  return;
	}
	int count = 0;
	while (_mutex && (count < MAX_SPIN)) {
	  _MM_PAUSE;
	  count++;
	}
	if (count == MAX_SPIN) {
	  yieldProcessor();
	}
      }
    }

    // Is this system a multiprocessor?
    inline bool onMultiprocessor (void) {
      static CPUInfo cpuInfo;
      return (cpuInfo.getNumProcessors() > 1);
    }

    inline void yieldProcessor (void) {
#if defined(_WIN32)
      Sleep(0);
#elif defined(__SVR4)
      thr_yield();
#else
      sched_yield();
#endif
    }

    enum { MAX_SPIN_LIMIT = 1024 };
  };

  typedef SpinLockType SpinLock;
  
}

#endif

#endif // _SPINLOCK_H_
