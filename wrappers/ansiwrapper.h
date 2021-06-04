/* -*- C++ -*- */

#ifndef HL_ANSIWRAPPER_H
#define HL_ANSIWRAPPER_H

#include <assert.h>
#include <string.h>
#include <stddef.h>
#include <cstdint>
#include <climits>

#ifndef _WIN32
#include <stdalign.h>
#endif

/*
 * @class ANSIWrapper
 * @brief Provide ANSI C behavior for malloc & free.
 *
 * Implements all prescribed ANSI behavior, including zero-sized
 * requests & aligned request sizes to a double word (or long word).
 */

namespace HL {

  template <class SuperHeap>
  class ANSIWrapper : public SuperHeap {
  private:
    static constexpr int gcd(int a, int b) {
      return (b == 0) ? a : gcd(b, a % b);
    }

  public:
  
    ANSIWrapper() {
      static_assert(gcd(SuperHeap::Alignment, 8) == 8, "Alignment mismatch");
    }
    using SuperHeap::SuperHeap;

    inline void * malloc (size_t sz) {
#if !defined(HL_NO_MALLOC_SIZE_CHECKS)
      static constexpr int alignment = 16; // safe for all platforms
      if (sz < alignment) {
      	sz = alignment;
      }
      // Prevent integer underflows. This maximum should (and
      // currently does) provide more than enough slack to compensate for any
      // rounding below (in the alignment section).
      if (sz >> (sizeof(size_t) * CHAR_BIT - 1)) {
	return 0;
      }
      // Enforce alignment requirements: round up allocation sizes if needed.
      // Enforce alignment.
      sz = (sz + alignment - 1UL) &
	~(alignment - 1UL);
#endif
      auto * ptr = SuperHeap::malloc (sz);
      return ptr;
    }
 
    inline void free (void * ptr) {
      if (ptr != 0) {
	SuperHeap::free (ptr);
      }
    }

    inline void free_sized (void * ptr, size_t sz) {
      if (ptr != 0) {
	SuperHeap::free_sized (ptr, sz);
      }
    }
    
    inline void * calloc (size_t s1, size_t s2) {
      auto * ptr = (char *) malloc (s1 * s2);
      if (ptr) {
      	memset (ptr, 0, s1 * s2);
      }
      return (void *) ptr;
    }
  
    inline void * realloc (void * ptr, const size_t sz) {
      if (ptr == 0) {
      	return malloc (sz);
      }
      if (sz == 0) {
      	free (ptr);
      	return 0;
      }

      auto objSize = getSize (ptr);
      if (objSize == sz) {
    	return ptr;
      }

      // Allocate a new block of size sz.
      auto * buf = malloc (sz);

      // Copy the contents of the original object
      // up to the size of the new block.

      auto minSize = (objSize < sz) ? objSize : sz;
      if (buf) {
	memcpy (buf, ptr, minSize);
      }

      // Free the old block.
      free (ptr);
      return buf;
    }
  
    inline size_t getSize (void * ptr) {
      if (ptr) {
	return SuperHeap::getSize (ptr);
      } else {
	return 0;
      }
    }
  };
}

#endif
