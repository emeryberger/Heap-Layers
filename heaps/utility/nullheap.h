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

#ifndef HL_NULLHEAP_H
#define HL_NULLHEAP_H

#include <assert.h>

/**
 * @class NullHeap
 * @brief A source heap that does nothing.
 */

namespace HL {

  template <class SuperHeap>
  class NullHeap : public SuperHeap {
  public:
    inline void * malloc (size_t) const { return 0; }
    inline void free (void *) const {}
    inline int remove (void *) const { return 0; }
    inline void clear (void) const {}
    inline size_t getSize (void *) const { return 0; }
  };

}

#endif
