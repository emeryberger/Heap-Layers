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


#ifndef HL_ONEHEAP_H
#define HL_ONEHEAP_H

//
// Wrap a single instance of a heap.
//

namespace HL {

  template <class SuperHeap>
  class OneHeap {
  public:
    OneHeap (void)
      : theHeap (getHeap())
    {}
    
    inline void * malloc (const size_t sz) {
      return theHeap->malloc (sz);
    }
    inline void free (void * ptr) {
      theHeap->free (ptr);
    }
    inline int remove (void * ptr) {
      return theHeap->remove (ptr);
    }
    inline void clear (void) {
      theHeap->clear();
    }
    inline size_t getSize (void * ptr) {
      return theHeap->getSize (ptr);
    }
    
    enum { Alignment = SuperHeap::Alignment };
    
  private:
    
    SuperHeap * theHeap;
    
    inline static SuperHeap * getHeap (void) {
      static SuperHeap theHeap;
      return &theHeap;
    }
  };
  
}

#endif
