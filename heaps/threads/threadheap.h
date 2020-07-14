/* -*- C++ -*- */

#ifndef HL_THREADHEAP_H
#define HL_THREADHEAP_H

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

#include "threads/cpuinfo.h"

#if !defined(_WIN32)
#include <pthread.h>
#endif

/*

  A ThreadHeap comprises NumHeaps "per-thread" heaps.

  To pick a per-thread heap, the current thread id is hashed (mod NumHeaps).

  malloc gets memory from its hashed per-thread heap.
  free returns memory to its hashed per-thread heap.

  (This allows the per-thread heap to determine the return
  policy -- 'pure private heaps', 'private heaps with ownership',
  etc.)

  NB: We assume that the thread heaps are 'locked' as needed.  */

namespace HL {

  template <int NumHeaps, class PerThreadHeap>
  class ThreadHeap : public PerThreadHeap {
  public:

    enum { Alignment = PerThreadHeap::Alignment };

    inline void * malloc (size_t sz) {
      auto tid = Modulo<NumHeaps>::mod (CPUInfo::getThreadId());
      assert (tid >= 0);
      assert (tid < NumHeaps);
      return getHeap(tid)->malloc (sz);
    }

    inline void free (void * ptr) {
      auto tid = Modulo<NumHeaps>::mod (CPUInfo::getThreadId());
      assert (tid >= 0);
      assert (tid < NumHeaps);
      getHeap(tid)->free (ptr);
    }

    inline size_t getSize (void * ptr) {
      auto tid = Modulo<NumHeaps>::mod (CPUInfo::getThreadId());
      assert (tid >= 0);
      assert (tid < NumHeaps);
      return getHeap(tid)->getSize (ptr);
    }

    
  private:

    // Access the given heap within the buffer.
    inline PerThreadHeap * getHeap (unsigned int index) {
      int ind = (int) index;
      assert (ind >= 0);
      assert (ind < NumHeaps);
      return &ptHeaps[ind];
    }

    PerThreadHeap ptHeaps[NumHeaps];

  };

}


#endif
