/* -*- C++ -*- */

#ifndef HL_ADDHEAP_H
#define HL_ADDHEAP_H

/*

  Heap Layers: An Extensible Memory Allocation Infrastructure
  
  Copyright (C) 2000-2020 by Emery Berger
  http://www.emeryberger.com
  emery@cs.umass.edu
  
  Heap Layers is distributed under the terms of the Apache 2.0 license.

  You may obtain a copy of the License at
  http://www.apache.org/licenses/LICENSE-2.0

*/

// Reserve space for a class in the head of every allocated object.

#include <assert.h>
#include "utility/lcm.h"

namespace HL {

  template <class Add, class SuperHeap>
  class AddHeap : public SuperHeap {
  public:

    inline void * malloc (size_t sz) {
      void * ptr = SuperHeap::malloc (sz + HeaderSize);
      void * newPtr = (char *) ptr + HeaderSize;
      return newPtr;
    }

    inline void free (void * ptr) {
      SuperHeap::free (getOriginal(ptr));
    }

    inline size_t getSize (void * ptr) {
      return SuperHeap::getSize (getOriginal(ptr));
    }

  private:

    inline void * getOriginal (void * ptr) {
      void * origPtr = (void *) ((char *) ptr - HeaderSize);
      return origPtr;
    }

    // A size that preserves existing alignment restrictions.
    // Beware: can seriously increase size requirements.
    enum { HeaderSize = lcm<(int) SuperHeap::Alignment, sizeof(Add)>::value };

  };

}
#endif
