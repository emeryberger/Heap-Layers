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

#ifndef HL_SPINLOCKOLD_H
#define HL_SPINLOCKOLD_H

#if defined(unix)
#include <sched.h>
#endif

#if defined(__SVR4)
#include <thread.h>
#endif

#if defined(__sgi)
#include <mutex.h>
#endif

#if defined(__APPLE__)
#include <libkern/OSAtomic.h>
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


#if defined(__SUNPRO_CC)
// x86-interchange.il, x86_64-interchange.il contributed by Markus Bernhardt.
extern "C" size_t MyInterlockedExchange (size_t * oldval,
						size_t newval);
#endif

#if defined(_WIN32) && !defined(_WIN64)

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif

// NOTE: Below is the new "pause" instruction, which is inocuous for
// previous architectures, but crucial for Intel chips with
// hyperthreading.  See
// http://www.usenix.org/events/wiess02/tech/full_papers/nakajima/nakajima.pdf
// for discussion.
// On PowerPC `or Rx Rx Rx` is a nop, but may have extra effects like setting
// priority etc. `or r27 r27 r27` is an equivanent of "pause", and in ISA 2.06+
// has an alias `yield`. For examples of usage, see Boost libfiber, catch2, seqan3.
// https://stackoverflow.com/questions/5425506/equivalent-of-x86-pause-instruction-for-ppc
// https://utcc.utoronto.ca/~cks/space/blog/tech/PowerPCInstructionOddity?showcomments

#define _MM_PAUSE  {__asm{_emit 0xf3};__asm {_emit 0x90}}
#include <windows.h>

#elif defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))

#define _MM_PAUSE  { asm (".byte 0xf3; .byte 0x90" : : : "memory"); }

#elif defined(__POWERPC__)

#define _MM_PAUSE  { __asm__ volatile ("or r27,r27,r27"); }

#else

#define _MM_PAUSE

#endif // defined(_WIN32) && !defined(_WIN64)

//extern volatile bool anyThreadCreated;

namespace HL {

  class SpinLockType {
  private:

    enum { UNLOCKED = 0, LOCKED = 1 };

  public:
  
    SpinLockType (void)
#if defined(__APPLE__)
      : mutex (OS_SPINLOCK_INIT)
#else
	: mutex (UNLOCKED)
#endif
    {}
  
    ~SpinLockType()
    {}

    inline void lock() {
      if (true) { // anyThreadCreated) {
	if (MyInterlockedExchange (const_cast<size_t *>(&mutex), LOCKED)
	    != UNLOCKED) {
	  contendedLock();
	}
      } else {
	mutex = LOCKED;
      }
    }



    inline void unlock() {
      if (true) { // anyThreadCreated) {
#if defined(_WIN32) && !defined(_WIN64)
	__asm {}
#elif defined(__GNUC__)
	asm volatile ("" : : : "memory");
#endif
      }
      mutex = UNLOCKED;
    }


#if !defined(__SUNPRO_CC)
    inline static size_t MyInterlockedExchange (size_t *, size_t); 
#endif

  private:

#if 0 // defined(__APPLE__)
    OSSpinLock mutex;

#else

    NO_INLINE
    void contendedLock() {
      const int MAX_SPIN = 1000;
      while (true) {
	if (MyInterlockedExchange (const_cast<size_t *>(&mutex), LOCKED)
	    == UNLOCKED) {
	  return;
	}
	int count = 0;
	while ((mutex == LOCKED) && (count < MAX_SPIN)) {
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

    volatile size_t mutex;
#endif

  };

}

// Atomically:
//   retval = *oldval;
//   *oldval = newval;
//   return retval;

#if !defined(__SUNPRO_CC) // && !defined(__APPLE__)
inline size_t 
HL::SpinLockType::MyInterlockedExchange (size_t * oldval,
					 size_t newval)
{
#if defined(_WIN32) && defined(_MSC_VER)
  return InterlockedExchange ((volatile LONG *) oldval, newval);

  //#elif (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__) > 40100
  //  return __sync_val_compare_and_swap(oldval, *oldval, newval);

#elif defined(__sparc)
  asm volatile ("swap [%1],%0"
		:"=r" (newval)
		:"r" (oldval), "0" (newval)
		: "memory");
  
#elif defined(__i386__)
  asm volatile ("lock; xchgl %0, %1"
		: "=r" (newval)
		: "m" (*oldval), "0" (newval)
		: "memory");

#elif defined(__sgi)
  newval = test_and_set (oldval, newval);

#elif defined(__x86_64__)
  // Contributed by Kurt Roeckx.
  asm volatile ("lock; xchgq %0, %1"
		: "=r" (newval)
		: "m" (*oldval), "0" (newval)
		: "memory");

#elif defined(__ppc) || defined(__powerpc__) || defined(PPC) || defined(__POWERPC__)
  // PPC assembly contributed by Maged Michael.
  int ret; 
  asm volatile ( 
		"La..%=0:    lwarx %0,0,%1 ;" 
		"      cmpw  %0,%2;" 
		"      beq La..%=1;" 
		"      stwcx. %2,0,%1;" 
		"      bne- La..%=0;" 
		"La..%=1:    isync;" 
                : "=&r"(ret) 
                : "r"(oldval), "r"(newval) 
                : "cr0", "memory"); 
  return ret;

#elif defined(__arm__)
  // Contributed by Bo Granlund.
  long result;
  asm volatile (
		"\n\t"
		"swp     %0,%2,[%1] \n\t"
		""
		: "=&r"(result)
		: "r"(oldval), "r"(newval)
		: "memory");
  return (result);
#elif defined(__APPLE__)
  size_t oldValue = *oldval;
  bool swapped = OSAtomicCompareAndSwapLongBarrier (oldValue, newval, (volatile long *) oldval);
  if (swapped) {
    return newval;
  } else {
    return oldValue;
  }
#else
#error "No spin lock implementation is available for this platform."
#endif
  return newval;
}


#endif

#endif // _SPINLOCK_H_
