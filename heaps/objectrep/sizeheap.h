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

#include "wrappers/mallocinfo.h"
#include "heaps/objectrep/addheap.h"
#include "utility/gcd.h"

namespace HL {

  /**
   * @class SizeHeap
   * @brief Allocates extra room for the size of an object.
   */

  template <class SuperHeap>
  class SizeHeap : public SuperHeap {

  private:
    struct freeObject {
      size_t _sz;
      size_t _magic;
      //      char _buf[HL::MallocInfo::Alignment];
    };

    enum { MAGIC_NUMBER = 0xCAFEBABE };
    
  public:

    enum { Alignment = gcd<(int) SuperHeap::Alignment,
	   (int) sizeof(freeObject)>::value };

    virtual ~SizeHeap (void) {}

    inline void * malloc (size_t sz) {
      freeObject * p = (freeObject *) SuperHeap::malloc (sz + sizeof(freeObject));
      p->_sz = sz;
      p->_magic = MAGIC_NUMBER;
      return (void *) (p + 1);
    }

    inline void free (void * ptr) {
      if (getHeader(ptr)->_magic == MAGIC_NUMBER) {
	// Probably one of our objects.
	SuperHeap::free (getHeader(ptr));
      }
    }

    inline static size_t getSize (const void * ptr) {
      if (getHeader(ptr)->_magic == MAGIC_NUMBER) {
	size_t size = getHeader(ptr)->_sz;
	return size;
      } else {
	// Probably not one of our objects.
	return 0;
      }
    }

  private:

    inline static void setSize (void * ptr, size_t sz) {
      assert (getHeader(ptr)->_magic == MAGIC_NUMBER);
      getHeader(ptr)->_sz = sz;
    }

    inline static freeObject * getHeader (const void * ptr) {
      return ((freeObject *) ptr - 1);
    }

  };

}

#endif
