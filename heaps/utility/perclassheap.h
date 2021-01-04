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

#ifndef HL_PERCLASSHEAP_H
#define HL_PERCLASSHEAP_H

#include <new>

/**
 * @class PerClassHeap
 * @brief Enable the use of one heap for all class memory allocation.
 * 
 * This class contains one instance of the SuperHeap argument.  The
 * example below shows how to make a subclass of Foo that uses a
 * FreelistHeap to manage its memory, overloading operators new and
 * delete.
 * 
 * <TT>
 *   class NewFoo : public Foo, PerClassHeap<FreelistHeap<MallocHeap> > {};
 * </TT>
 */

namespace HL {

  template <class SuperHeap>
    class PerClassHeap {
  public:
    inline void * operator new (size_t sz) {
      return getHeap()->malloc (sz);
    }
    inline void operator delete (void * ptr) {
      getHeap()->free (ptr);
    }
    inline void * operator new[] (size_t sz) {
      return getHeap()->malloc (sz);
    }
    inline void operator delete[] (void * ptr) {
      getHeap()->free (ptr);
    }
    // For some reason, g++ needs placement new to be overridden
    // as well, at least in conjunction with use of the STL.
    // Otherwise, this should be superfluous.
    inline void * operator new (size_t, void * p) { return p; }
    inline void * operator new[] (size_t, void * p) { return p; }

  private:
    inline static SuperHeap * getHeap (void) {
      static SuperHeap theHeap;
      return &theHeap;
    }
  };

}

#endif
