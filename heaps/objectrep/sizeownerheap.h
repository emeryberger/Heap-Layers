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

#ifndef HL_SIZEOWNERHEAP_H_
#define HL_SIZEOWNERHEAP_H_

#include <assert.h>

#include "addheap.h"

/**
 * @class SizeOwnerHeap
 * @brief Adds object size and owner heap information.
 */

namespace HL {

template <class Heap>
class SizeOwner {
public:
  union tag {
    struct {
      size_t size;
      Heap * owner;
    } s;
    double dummy;
  };
};

template <class Super>
class SizeOwnerHeap : public AddHeap<SizeOwner<Super>, Super> {
private:

  typedef AddHeap<SizeOwner<Super>, Super> SuperHeap;

public:

  inline void * malloc (size_t sz) {
    void * ptr = SuperHeap::malloc (sz);
    // Store the requested size.
    SizeOwner<Super> * so = (SizeOwner<Super> *) ptr;
    so->s.size = sz;
    so->s.owner = this;
    // Store the owner.
    return (void *) (so + 1);
  }

  inline void free (void * ptr) {
    void * origPtr = (void *) ((SizeOwner<Super> *) ptr - 1);
    SuperHeap::free (origPtr);
  }

  static inline Super * owner (void * ptr) {
    SizeOwner<Super> * so = (SizeOwner<Super> *) ptr - 1;
    return so->s.owner;
  }

  static inline size_t size (void * ptr) {
    SizeOwner<Super> * so = (SizeOwner<Super> *) ptr - 1;
    return so->s.size;
  }
};

}

#endif
