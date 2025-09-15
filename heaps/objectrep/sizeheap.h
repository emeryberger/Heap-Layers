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

#ifndef HL_SIZEHEAP_H
#define HL_SIZEHEAP_H

/**
 * @file sizeheap.h
 * @brief Contains UseSizeHeap and SizeHeap.
 */

#include <assert.h>
#include "heaps/objectrep/headerheap.h"

namespace HL {

  /**
   * @class SizeHeap
   * @brief Allocates extra room for the size of an object.
   */

  enum { MAGIC_NUMBER = 0xCAFEBABE };

  struct sizeHeap_t {
    size_t _sz;
#ifndef NDEBUG
    size_t _magic;
#endif
  };

  inline void sizeHeap_func(sizeHeap_t* headerObjectPtr, size_t sz, void *ptr) {
    headerObjectPtr->_sz = sz;
#ifndef NDEBUG
    headerObjectPtr->_magic = MAGIC_NUMBER;
#endif
  }

  template <class SuperHeap>
  class SizeHeap : public HeaderHeap<SuperHeap, sizeHeap_t, sizeHeap_func> {
      using HeaderHeapT = HeaderHeap<SuperHeap, sizeHeap_t, sizeHeap_func>;
    public:
      inline static size_t getSize (const void * ptr) {
        assert(HeaderHeapT::getHeader(ptr)->_magic == MAGIC_NUMBER);
        size_t size = HeaderHeapT::getHeader(ptr)->_sz;
        return size;
      }
  };
}

#endif
