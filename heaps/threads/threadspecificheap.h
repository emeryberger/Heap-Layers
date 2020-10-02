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

namespace HL {

  template <class PerThreadHeap>
  class ThreadSpecificHeap {
  public:

    ThreadSpecificHeap (void)
    {
      // Initialize the heap exactly once.
      pthread_once (&(getOnce()), initializeHeap);
    }

    virtual ~ThreadSpecificHeap()
    {
    }

    inline void * malloc (size_t sz) {
      return getHeap()->malloc (sz);
    }

    inline void free (void * ptr) {
      PerThreadHeap * heap =
	(PerThreadHeap *) pthread_getspecific (getHeapKey());
      if (heap) heap->free (ptr);
    }

    inline size_t getSize (void * ptr) {
      return getHeap()->getSize(ptr);
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

    static void deleteHeap (void *) {
      PerThreadHeap * heap = getHeap();
      heap->~PerThreadHeap();
      HL::MmapWrapper::unmap (heap, sizeof(PerThreadHeap));
      pthread_setspecific (getHeapKey(), 0);
    }

    // Access the given heap.
    static PerThreadHeap * getHeap() {
      PerThreadHeap * heap =
	(PerThreadHeap *) pthread_getspecific (getHeapKey());
      if (heap == NULL)  {
	// Grab some memory from a source, initialize the heap inside,
	// and store it in the thread-local area.
	void * buf = HL::MmapWrapper::map (sizeof(PerThreadHeap));
	heap = new (buf) PerThreadHeap;
	pthread_setspecific (getHeapKey(), (void *) heap);
      }
      return heap;
    }
  };

}

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

#endif

#endif
