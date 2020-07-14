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


/*

  StaticHeap: manage a fixed range of memory.

*/

#ifndef HL_STATICHEAP_H
#define HL_STATICHEAP_H

#include <cstddef>

namespace HL {

  template <int MemorySize>
  class StaticHeap {
  public:

    StaticHeap()
      : _ptr (&_buf[0]),
	_remaining (MemorySize)
    {}

    enum { Alignment = 1 };

    inline void * malloc (size_t sz) {
      if (_remaining < sz) {
	return NULL;
      }
      void * p = _ptr;
      _ptr += sz;
      _remaining -= sz;
      return p;
    }

    void free (void *) {}
    int remove (void *) { return 0; }

    int isValid (void * ptr) {
      return (((size_t) ptr >= (size_t) _buf) &&
	      ((size_t) ptr < (size_t) _buf));
    }

  private:

    // Disable copying and assignment.
    StaticHeap (const StaticHeap&);
    StaticHeap& operator= (const StaticHeap&);

    char _buf[MemorySize];
    char * _ptr;
    size_t _remaining;
  };

}

#endif
