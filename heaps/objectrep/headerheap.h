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

#ifndef HL_HEADERHEAP_H
#define HL_HEADERHEAP_H

/**
 * @class HeaderHeap
 * @brief Generic header heap. User provides only the header struct and function to set the header.
 * @author André Costa
 */

#include <assert.h>

namespace HL {

  template <class SuperHeap, class headerObject,
    void (*headerFunction)(headerObject*, size_t sz, void *)>
  class HeaderHeap : public SuperHeap {
    public:
      ~HeaderHeap () {}

      enum {Alignment = gcd<(int)SuperHeap::Alignment,(int)sizeof(headerObject)>::value};

      inline void * malloc (size_t sz) {
        headerObject* headerObjectPtr = (headerObject*) SuperHeap::malloc(sz + sizeof(headerObject));
        void *ptr = (void *) ((uintptr_t)ptr + sizeof(headerObject)); //ptr that "user" heap will see
        headerFunction(headerObjectPtr, sz, ptr);
        return (void *) (headerObjectPtr + 1);
      }

      inline void free (void * ptr) {
	      SuperHeap::free (getHeader(ptr));
      }

      inline static headerObject* getHeader(const void * ptr) {
        return (headerObject*) ((uintptr_t)ptr - sizeof(headerObject));
      }
  };

} //namespace HL

#endif
