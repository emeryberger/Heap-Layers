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

#ifndef HL_KINGSLEYHEAP_H
#define HL_KINGSLEYHEAP_H

#include "utility/ilog2.h"
#include "heaps/combining/strictsegheap.h"

/**
 * @file kingsleyheap.h
 * @brief Classes to implement a Kingsley (power-of-two, segregated fits) allocator.
 */

/**
 * @namespace Kingsley
 * @brief Functions to implement KingsleyHeap.
 */

namespace Kingsley {

  inline size_t class2Size (const int i) {
    auto sz = (size_t) (1ULL << (i+3));
    return sz;
  }

  inline int size2Class (const size_t sz) {
    auto cl = (int) HL::ilog2 ((sz < 8) ? 8 : sz) - 3;
    return cl;
  }

  enum { NUMBINS = 29 };

}

/**
 * @class KingsleyHeap
 * @brief The Kingsley-style allocator.
 * @param PerClassHeap The heap to use for each size class.
 * @param BigHeap The heap for "large" objects.
 * @see Kingsley
 */

namespace HL {

template <class PerClassHeap, class BigHeap>
  class KingsleyHeap :
   public StrictSegHeap<Kingsley::NUMBINS,
                        Kingsley::size2Class,
                        Kingsley::class2Size,
                        PerClassHeap,
                        BigHeap> {};

}

#endif
