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

#ifndef HL_TRYHEAP_H
#define HL_TRYHEAP_H

#include <cstddef>

namespace HL {

  template <class Heap1, class Heap2>
  class TryHeap : public Heap2 {
  public:

    TryHeap (void)
    {}

    inline void * malloc (size_t sz) {
      void * ptr = heap1.malloc (sz);
      if (ptr == NULL)
        ptr = Heap2::malloc (sz);
      return ptr;
    }

    inline void free (void * ptr) {
      heap1.free (ptr);
    }

  private:
    Heap1 heap1;
  };

}

#endif
