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

#ifndef HL_LEAMALLOCHEAP_H
#define HL_LEAMALLOCHEAP_H

#include <cstdlib>

#include "wrappers/mallocinfo.h"

/**
 * @class LeaMallocHeap
 * @brief A "source heap" that uses the Lea allocator.
 */

extern "C" void * dlmalloc (size_t);
extern "C" void   dlfree (void *);
extern "C" size_t dlmalloc_usable_size (void *);

namespace HL {

  class LeaMallocHeap {
  public:
    enum { Alignment = MallocInfo::Alignment };

    inline void * malloc (size_t sz) {
      void * ptr = dlmalloc (sz);
      return ptr;
    }

    inline void free (void * p) {
      dlfree (p);
    }

    inline size_t getSize (const void * p) {
      return dlmalloc_usable_size ((void *) p);
    }
  };

}

#endif
