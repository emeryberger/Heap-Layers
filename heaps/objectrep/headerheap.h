/* -*- C++ -*- */

/*

  Heap Layers: An Extensible Memory Allocation Infrastructure

  Copyright (C) 2000-2024 by Emery Berger
  http://www.emeryberger.com
  emery@cs.umass.edu

  Heap Layers is distributed under the terms of the Apache 2.0 license.

  You may obtain a copy of the License at
  http://www.apache.org/licenses/LICENSE-2.0

*/

#ifndef HL_HEADERHEAP_H
#define HL_HEADERHEAP_H

/**
 * @file headerheap.h
 * @brief A generic heap layer that prepends a header to each allocation.
 *
 * HeaderHeap reserves space for a Header object before each allocation
 * and provides getHeader() to recover it from a user pointer. Subclasses
 * override malloc/free to initialize and validate the header.
 *
 * Compare to AddHeap, which reserves space using lcm-based sizing
 * but does not provide getHeader() or initialization hooks.
 */

#include "utility/gcd.h"

namespace HL {

  template <class Header, class SuperHeap>
  class HeaderHeap : public SuperHeap {
  public:

    enum { Alignment = gcd<(int) SuperHeap::Alignment,
	   (int) sizeof(Header)>::value };

    inline void * malloc (size_t sz) {
      Header * p = (Header *) SuperHeap::malloc (sz + sizeof(Header));
      return (void *) (p + 1);
    }

    inline void free (void * ptr) {
      SuperHeap::free (getHeader(ptr));
    }

    inline static Header * getHeader (const void * ptr) {
      return ((Header *) ptr - 1);
    }
  };

}

#endif
