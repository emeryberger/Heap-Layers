/* -*- C++ -*- */

#ifndef HL_THREADSPECIFICHEAP_H
#define HL_THREADSPECIFICHEAP_H

/*

  Heap Layers: An Extensible Memory Allocation Infrastructure
  
  Copyright (C) 2000-2020 by Emery Berger
  http://www.emeryberger.com
  emery@cs.umass.edu
  
  Heap Layers is distributed under the terms of the Apache 2.0 license.

  You may obtain a copy of the License at
  http://www.apache.org/licenses/LICENSE-2.0

*/

#include <assert.h>
#include <new>

#if !defined(_WIN32) // not implemented for Windows

#include <pthread.h>

#include "wrappers/mmapwrapper.h"

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#endif

// Compute the version of gcc we're compiling with (if any).
#define GCC_VERSION (__GNUC__ * 10000	    \
                     + __GNUC_MINOR__ * 100 \
                     + __GNUC_PATCHLEVEL__)

#if (((defined(GCC_VERSION) && (GCC_VERSION >= 30300)) &&	\
      !defined(__SVR4))						\
  && !defined(__APPLE__) \
     || defined(__SUNPRO_CC)					\
     || defined(__FreeBSD__))
#define USE_THREAD_KEYWORD 1
#else
#define USE_THREAD_KEYWORD 0
#endif

namespace HL {

#if USE_THREAD_KEYWORD
#if !defined(INITIAL_EXEC_ATTR)
#define INITIAL_EXEC_ATTR __attribute__((tls_model ("initial-exec")))
#endif

  template <class PerThreadHeap>
  class ThreadSpecificHeap {
  private:
    alignas(16) static __thread char heapbuf[sizeof(PerThreadHeap)];
    static __thread PerThreadHeap * heap;
  public:

    ThreadSpecificHeap()
    {
    }

    ~ThreadSpecificHeap() {
      if (heap) {
	heap->~PerThreadHeap();
      }
    }
    
    inline void * malloc(size_t sz) {
      if (heap == nullptr) {
	heap = new (heapbuf) PerThreadHeap();
      }
      return heap->malloc(sz);
    }

    inline void free(void * ptr) {
      if (heap == nullptr) {
	heap = new (heapbuf) PerThreadHeap();
      }
      heap->free(ptr);
    }

    inline void * memalign(size_t alignment, size_t sz) {
      return heap->memalign(alignment, sz);
    }
    
    inline size_t getSize(void * ptr) {
      return heap->getSize(ptr);
    }

    enum { Alignment = PerThreadHeap::Alignment };
  };

  template <class PerThreadHeap>
  alignas(16) __thread char ThreadSpecificHeap<PerThreadHeap>::heapbuf[sizeof(PerThreadHeap)] INITIAL_EXEC_ATTR;

  template <class PerThreadHeap>
  __thread PerThreadHeap * ThreadSpecificHeap<PerThreadHeap>::heap INITIAL_EXEC_ATTR = nullptr;


#else // USE_THREAD_KEYWORD
  
  template <class PerThreadHeap>
  class ThreadSpecificHeap {
  public:

    ThreadSpecificHeap()
    {
      // Initialize the heap exactly once.
      pthread_once (&(getOnce()), initializeHeap);
    }

    inline void * malloc (size_t sz) {
      return getHeap()->malloc (sz);
    }

    inline void free (void * ptr) {
      getHeap()->free (ptr);
    }

    inline size_t getSize (void * ptr) {
      return getHeap()->getSize(ptr);
    }

    inline void * memalign(size_t alignment, size_t sz) {
      return getHeap()->memalign(alignment, sz);
    }
    
    enum { Alignment = PerThreadHeap::Alignment };

  private:

    static void initializeHeap() {
      getHeap();
    }

    static pthread_key_t& getHeapKey() {
      static pthread_key_t heapKey;
      static int r = pthread_key_create (&heapKey, deleteHeap);
      return heapKey;
    }

    static pthread_once_t& getOnce() {
      static pthread_once_t initOnce = PTHREAD_ONCE_INIT;
      return initOnce;
    }

    static void deleteHeap (void * heap) {
      ((PerThreadHeap *) heap)->~PerThreadHeap();
      HL::MmapWrapper::unmap (heap, sizeof(PerThreadHeap));
    }

    // Access the given heap.
    static PerThreadHeap * getHeap() {
      PerThreadHeap * heap =
	(PerThreadHeap *) pthread_getspecific (getHeapKey());
      if (heap == nullptr)  {
	// Grab some memory from a source, initialize the heap inside,
	// and store it in the thread-local area.
	void * buf = HL::MmapWrapper::map (sizeof(PerThreadHeap));
	heap = new (buf) PerThreadHeap;
	pthread_setspecific (getHeapKey(), (void *) heap);
      }
      return heap;
    }
  };

#endif
  
}

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

#endif

#endif
