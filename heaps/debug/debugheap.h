/* -*- C++ -*- */


#ifndef HL_DEBUGHEAP_H_
#define HL_DEBUGHEAP_H_

#include <assert.h>
#include <stdlib.h>

/**
 *
 *
 */

namespace HL {

  template <class Super,
	    char freeChar = 'F'>
  class DebugHeap : public Super {
  private:

    enum { CANARY = 0xdeadbeef };

  public:

    // Fill with A's.
    inline void * malloc (size_t sz) {
      // Add a guard area at the end.
      void * ptr;
      ptr = Super::malloc (sz + sizeof(unsigned long));
      if (ptr == NULL) {
	return NULL;
      }
      size_t realSize = Super::getSize (ptr);
      assert (realSize >= sz);
      for (unsigned long i = 0; i < realSize; i++) {
        ((char *) ptr)[i] = 'A';
      }
      unsigned long * canaryLocation =
	(unsigned long *) ((char *) ptr + realSize - sizeof(unsigned long));
      *canaryLocation = (unsigned long) CANARY;
      return ptr;
    }

    // Fill with F's.
    inline void free (void * ptr) {
      size_t realSize = Super::getSize(ptr);
      // Check for the canary.
      unsigned long * canaryLocation =
	(unsigned long *) ((char *) ptr + realSize - sizeof(unsigned long));
      unsigned long storedCanary = *canaryLocation;
      if (storedCanary != CANARY) {
        abort();
      }
      for (unsigned int i = 0; i < realSize; i++) {
        ((char *) ptr)[i] = freeChar;
      }
      Super::free (ptr);
    }
  };

}

#endif
