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

#ifndef HL_SIZEHEAP_H
#define HL_SIZEHEAP_H

/**
 * @file sizeheap.h
 * @brief Contains SizeHeap.
 */

#include <assert.h>

#include "heaps/objectrep/headerheap.h"
#include "utility/cpp23compat.h"

namespace HL {

  struct SizeHeapHeader {
    size_t _sz;
    size_t _magic;
  };

  /**
   * @class SizeHeap
   * @brief Allocates extra room for the size of an object.
   *
   * Uses HeaderHeap to prepend a header containing the requested size
   * and a magic number for validation.
   */

  template <class SuperHeap>
  class SizeHeap : public HeaderHeap<SizeHeapHeader, SuperHeap> {

  private:

    using Base = HeaderHeap<SizeHeapHeader, SuperHeap>;

    enum { MAGIC_NUMBER = 0xCAFEBABE };

  public:

    virtual ~SizeHeap (void) {}

    inline void * malloc (size_t sz) {
      void * ptr = Base::malloc (sz);
      Base::getHeader(ptr)->_sz = sz;
      Base::getHeader(ptr)->_magic = MAGIC_NUMBER;
      return ptr;
    }

    inline void free (void * ptr) {
      if (HL_EXPECT_TRUE(Base::getHeader(ptr)->_magic == MAGIC_NUMBER)) HL_LIKELY {
	// Probably one of our objects.
	Base::free (ptr);
      }
    }

    inline static size_t getSize (const void * ptr) {
      if (HL_EXPECT_TRUE(Base::getHeader(ptr)->_magic == MAGIC_NUMBER)) HL_LIKELY {
	size_t size = Base::getHeader(ptr)->_sz;
	return size;
      } else HL_UNLIKELY {
	// Probably not one of our objects.
	return 0;
      }
    }

  private:

    inline static void setSize (void * ptr, size_t sz) {
      assert (Base::getHeader(ptr)->_magic == MAGIC_NUMBER);
      Base::getHeader(ptr)->_sz = sz;
    }

  };

}

#endif
