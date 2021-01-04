// -*- C++ -*-

/*

  Heap Layers: An Extensible Memory Allocation Infrastructure
  
  Copyright (C) 2000-2020 by Emery Berger
  http://www.emeryberger.com
  emery@cs.umass.edu
  
  Heap Layers is distributed under the terms of the Apache 2.0 license.

  You may obtain a copy of the License at
  http://www.apache.org/licenses/LICENSE-2.0

*/

#ifndef HL_HYBRIDHEAP_H
#define HL_HYBRIDHEAP_H

#include <assert.h>

#include "heaplayers.h"

/**
 * @class HybridHeap
 * Objects no bigger than BigSize are allocated and freed to SmallHeap.
 * Bigger objects are passed on to the super heap.
 */

namespace HL {

  template <int BigSize, class SmallHeap, class BigHeap>
  class HybridHeap : public SmallHeap {
  public:

    HybridHeap (void)
    {
      static_assert(BigSize > 0, "Large object size must be positive.");
    }

    enum { Alignment = gcd<(int) SmallHeap::Alignment, (int) BigHeap::Alignment>::value };

    MALLOC_FUNCTION INLINE void * malloc (size_t sz) {
      void * ptr;
      if (sz <= BigSize) {
        ptr = SmallHeap::malloc (sz);
      } else {
        ptr = slowPath (sz);
      }
      if (SmallHeap::getSize(ptr) < sz) {
	assert(bm.getSize(ptr) >= sz);
      }
      assert (SmallHeap::getSize(ptr) >= sz);
      assert ((size_t) ptr % Alignment == 0);
      return ptr;
    }

    inline void free (void * ptr) {
      if (SmallHeap::getSize(ptr) <= BigSize) {
        SmallHeap::free (ptr);
      } else {
        bm.free (ptr);
      }
    }

    inline void clear (void) {
      bm.clear();
      SmallHeap::clear();
    }


  private:

    MALLOC_FUNCTION NO_INLINE
    void * slowPath (size_t sz) {
      return bm.malloc (sz);
    }


    BigHeap bm;
  };

}

#endif
