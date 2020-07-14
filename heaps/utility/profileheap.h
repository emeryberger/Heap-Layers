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


#ifndef HL_PROFILEHEAP_H
#define HL_PROFILEHEAP_H

#include <cstdio>

// Maintain & print memory usage info.
// Requires a superheap with the size() method (e.g., SizeHeap).

namespace HL {

  template <class SuperHeap, int HeapNumber>
  class ProfileHeap : public SuperHeap {
  public:
    
    ProfileHeap (void)
      : memRequested (0),
	maxMemRequested (0)
    {
    }
    
    ~ProfileHeap()
    {
      if (maxMemRequested > 0) {
	stats();
      }
    }
    
    inline void * malloc (size_t sz) {
      void * ptr = SuperHeap::malloc (sz);
      // Notice that we use the size reported by the allocator
      // for the object rather than the requested size.
      memRequested += SuperHeap::getSize(ptr);
      if (memRequested > maxMemRequested) {
	maxMemRequested = memRequested;
      }
      return ptr;
    }
    
    inline void free (void * ptr) {
      memRequested -= SuperHeap::getSize (ptr);
      SuperHeap::free (ptr);
    }
    
  private:
    void stats (void) {
      printf ("Heap: %d\n", HeapNumber);
      printf ("Max memory requested = %d\n", maxMemRequested);
      printf ("Memory still in use = %d\n", memRequested);
    }
    
    unsigned long memRequested;
    unsigned long maxMemRequested;
  };

}

#endif
