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

#ifndef HL_MALLOCHEAP_H
#define HL_MALLOCHEAP_H

#include <cstdlib>

#if defined(__SVR4)
extern "C" size_t malloc_usable_size (void *);
#elif defined(__APPLE__)
#include <malloc/malloc.h>
#elif defined(__linux__)
#include <malloc.h>
#else
extern "C" size_t malloc_usable_size (void *) throw ();
#endif

/**
 * @class MallocHeap
 * @brief A "source heap" that uses malloc and free.
 */

#include "wrappers/mallocinfo.h"


namespace HL {

  class MallocHeap {
  public:

    enum { Alignment = MallocInfo::Alignment };

    inline void * malloc (size_t sz) {
      return ::malloc (sz);
    }
  
    inline void free (void * ptr) {
      ::free (ptr);
    }

#if defined(_MSC_VER)
    inline size_t getSize (void * ptr) {
      return ::_msize (ptr);
    }
#elif defined(__APPLE__)
    inline size_t getSize (void * ptr) {
      return ::malloc_size (ptr);
    }
#elif defined(__GNUC__) && !defined(__SVR4)
    inline size_t getSize (void * ptr) {
      return ::malloc_usable_size (ptr);
    }
#endif
  
  };

}

#endif
