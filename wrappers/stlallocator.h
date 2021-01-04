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

#ifndef HL_STLALLOCATOR_H
#define HL_STLALLOCATOR_H

#include <cstdio>
#include <new>
#include <cstdlib>
#include <limits>

#include <memory> // STL

// Somewhere someone is defining a max macro (on Windows),
// and this is a problem -- solved by undefining it.

#if defined(max)
#undef max
#endif

using namespace std;

/**
 * @class STLAllocator
 * @brief An allocator adapter for STL.
 * 
 * This mixin lets you use any Heap Layers allocator as the allocator
 * for an STL container.
 *
 * Example:
 * <TT>
 *   typedef STLAllocator<int, MyHeapType> MyAllocator;<BR>
 *   list<int, MyAllocator> l;<BR>
 * </TT>
 */

namespace HL {

template <class T, class Super>
class STLAllocator : public Super {
public:

  typedef T value_type;
  typedef T * pointer;
  typedef const T * const_pointer;
  typedef T& reference;
  typedef const T& const_reference;
  typedef std::size_t size_type;
  typedef std::ptrdiff_t difference_type;
  typedef std::true_type propagate_on_container_move_assignment;

  template <class U>
  struct rebind {
    typedef STLAllocator<U,Super> other; 
  };

  pointer address (reference x) const {
    return &x;
  }

  const_pointer address (const_reference x) const {
    return &x;
  }

  STLAllocator() throw() {
  }

  STLAllocator (const STLAllocator& s) throw()
    : Super (s)
  {}

  virtual ~STLAllocator() throw() {
  }

  /// Make the maximum size be the largest possible object.
  size_type max_size() const
  {
    return std::numeric_limits<std::size_t>::max() / sizeof(T);
  }

#if defined(_WIN32)
  char * _Charalloc (size_type n) {
    return (char *) allocate (n);
  }
#endif

#if defined(__SUNPRO_CC)
  inline void * allocate (size_type n,
			  const void * = 0) {
    if (n) {
      return reinterpret_cast<void *>(Super::malloc (sizeof(T) * n));
    } else {
      return (void *) 0;
    }
  }
#else
  inline pointer allocate (size_type n,
			  const void * = 0) {
    if (n) {
      return reinterpret_cast<pointer>(Super::malloc (sizeof(T) * n));
    } else {
      return 0;
    }
  }
#endif

  inline void deallocate (void * p, size_type) {
    Super::free (p);
  }

  inline void deallocate (pointer p, size_type) {
    Super::free (p);
  }
  
#if __cplusplus >= 201103L  
  // Updated API from C++11 to work with move constructors
  template<class U, class... Args>
  void construct (U* p, Args&&... args) {
    new((void *) p) U(std::forward<Args>(args)...);
  }
  
  template<class U>
  void destroy (U* p) {
    p->~U();
  }

#else
  // Legacy API for pre-C++11
  void construct (pointer p, const T& val) { 
    new ((void *) p) T (val);
  }
  
  void destroy (pointer p) {
    ((T*)p)->~T();
  }
  
#endif

  template <class U> STLAllocator (const STLAllocator<U, Super> &) throw() 
  {
  }

};

  template <typename T, class S>
  inline bool operator!=(const STLAllocator<T,S>& a, const STLAllocator<T,S>& b) {
    return (&a != &b);
  }

  template <typename T, class S>
  inline bool operator==(const STLAllocator<T,S>& a, const STLAllocator<T,S>& b) {
    return (&a == &b);
  }


}


#endif
