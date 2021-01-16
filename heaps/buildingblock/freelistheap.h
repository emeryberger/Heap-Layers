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

#ifndef HL_FREELISTHEAP_H
#define HL_FREELISTHEAP_H

/**
 * @class FreelistHeap
 * @brief Manage freed memory on a linked list.
 * @warning This is for one "size class" only.
 *
 * Note that the linked list is threaded through the freed objects,
 * meaning that such objects must be at least the size of a pointer.
 */

#include <assert.h>
#include "utility/freesllist.h"

namespace HL {

  template <class SuperHeap>
  class FreelistHeap : public SuperHeap {
  public:

    inline void * malloc (size_t sz) {
      // Check the free list first.
      void * ptr = _freelist.get();
      // If it's empty, get more memory;
      // otherwise, advance the free list pointer.
      if (!ptr) {
        ptr = SuperHeap::malloc (sz);
      }
      return ptr;
    }

    inline void free (void * ptr) {
      if (!ptr) {
        return;
      }
      _freelist.insert (ptr);
    }

    inline void clear (void) {
      void * ptr;
      while ((ptr = _freelist.get())) {
        SuperHeap::free (ptr);
      }
    }

  private:

    FreeSLList _freelist;

  };

}

#endif
