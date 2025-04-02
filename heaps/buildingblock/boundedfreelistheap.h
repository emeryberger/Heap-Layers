/* -*- C++ -*- */

#ifndef HL_BOUNDEDFREELISTHEAP_H_
#define HL_BOUNDEDFREELISTHEAP_H_

// Beware -- this is for one "size class" only!!

#include <cstdlib>
#include "utility/freesllist.h"

template <int numObjects, class Super>
class BoundedFreeListHeap : public Super {
public:

  BoundedFreeListHeap()
    : nObjects (0)
  {}

  ~BoundedFreeListHeap()
  {
    clear();
  }

  inline void * malloc (size_t sz) {
    // Check the free list first.
    void * ptr = _freelist.get();
    if (!ptr) {
      assert(nObjects == 0);
      ptr = Super::malloc (sz);
    } else {
      nObjects--;
    }
    return ptr;
  }

  inline void free (void * ptr) {
    if (nObjects < numObjects) {
      // Add this object to the free list.
      _freelist.insert(ptr);
      nObjects++;
    } else {
      clear();
      //      Super::free (ptr);
    }
  }

  inline void clear (void) {
    // Delete everything on the free list.
    void * ptr;
    while ((ptr = _freelist.get()) != NULL) {
      Super::free(ptr);
      assert(nObjects-- > 0);
    }
    assert(nObjects == 0);
    nObjects = 0;
  }

private:

  int nObjects;
  FreeSLList _freelist;
};

#endif
