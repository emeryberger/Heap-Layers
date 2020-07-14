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

#ifndef HL_EXCEPTIONHEAP_H
#define HL_EXCEPTIONHEAP_H

#include <new>

// explicit throw lists are deprecated in C++11 and higher, and GCC
// complains about them
#if __cplusplus >= 201103L
#define HL_THROW_BAD_ALLOC
#else
class std::bad_alloc;
#define HL_THROW_BAD_ALLOC throw (std::bad_alloc)
#endif


namespace HL {

  template <class Super>
  class ExceptionHeap : public Super {
  public:
    inline void * malloc (size_t sz) HL_THROW_BAD_ALLOC {
      void * ptr = Super::malloc (sz);
      if (ptr == NULL) {
        throw new std::bad_alloc;
      }
      return ptr;
    }
  };


  template <class Super>
  class CatchExceptionHeap : public Super {
  public:
    inline void * malloc (size_t sz) {
      void * ptr;
      try {
	ptr = Super::malloc (sz);
      } catch (std::bad_alloc) {
	ptr = NULL;
      }
      return ptr;
    }
  };

}

#endif
