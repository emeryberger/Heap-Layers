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

#ifndef HL_EXACTLYONEHEAP_H
#define HL_EXACTLYONEHEAP_H

#include "utility/exactlyone.h"

namespace HL {

  template <class Heap>
  class ExactlyOneHeap : public HL::ExactlyOne<Heap> {
  public:

    enum { Alignment = Heap::Alignment };

    inline void * malloc (size_t sz) {
      return (*this)().malloc (sz);
    }
    inline void free (void * ptr) {
      (*this)().free (ptr);
    }
    inline void free (void * ptr, size_t sz) {
      (*this)().free (ptr, sz);
    }
    inline size_t getSize (void * ptr) {
      return (*this)().getSize (ptr);
    }
    inline int getClass (void * ptr) {
      return (*this)().getClass (ptr);
    }
    inline void clear() {
      (*this)().clear();
    }
    inline void lock() {
      (*this)().lock();
    }
    inline void unlock() {
      (*this)().unlock();
    }
  };

}

#endif
