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
// Note: Do NOT #define inline __forceinline - it breaks MSVC STL headers
#endif // !defined(NO_INLINE)

#else

#define INLINE inline
#define NO_INLINE

#endif // defined(_MSC_VER)


#if defined(_WIN32) && !defined(_WIN64)

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif

#endif

// Re-enabled PAUSE/YIELD hints for spin loops.
// Short PAUSE sequences are beneficial on modern x86 (the Skylake-X issue
// was with long PAUSE loops; a single PAUSE per spin iteration is fine).
// On ARM64, YIELD serves the same purpose.
#if defined(_WIN32)
#define _MM_PAUSE YieldProcessor()
#elif defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
#define _MM_PAUSE  { __asm__ __volatile__("pause" ::: "memory"); }
#elif defined(__GNUC__) && (defined(__aarch64__) || defined(__arm__))
#define _MM_PAUSE  { __asm__ __volatile__("yield" ::: "memory"); }
#else
#define _MM_PAUSE
#endif

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
      // Acquire semantics: operations after lock() cannot be reordered before
      if (_mutex.exchange(true, std::memory_order_acquire)) {
	contendedLock();
      }
    }

    inline bool didLock() {
      return !_mutex.exchange(true, std::memory_order_acquire);
    }

    inline void unlock() {
      // Release semantics: operations before unlock() cannot be reordered after
      _mutex.store(false, std::memory_order_release);
    }

  private:

    NO_INLINE
    void contendedLock() {
      // Exponential backoff: spin 1, 2, 4, 8, ... up to MAX_BACKOFF
      // iterations before yielding. Reduces cache-line bouncing under
      // contention while keeping uncontended latency low.
      enum { MAX_BACKOFF = 64 };
      int backoff = 1;
      while (true) {
	// Try to acquire.
	if (!_mutex.exchange(true, std::memory_order_acquire)) {
	  return;
	}
	// Spin with PAUSE, exponentially increasing spin count.
	for (int i = 0; i < backoff; i++) {
	  _MM_PAUSE;
	  if (!_mutex.load(std::memory_order_relaxed)) {
	    break;  // Lock might be available, try exchange again.
	  }
	}
	if (backoff < MAX_BACKOFF) {
	  backoff *= 2;
	} else {
	  yieldProcessor();
	  backoff = 1;  // Reset after yield.
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
