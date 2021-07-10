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

#pragma once
#ifndef SYSMALLOCHEAP_HPP
#define SYSMALLOCHEAP_HPP

namespace HL {

/**
 * @class SysMallocHeap
 * @brief Forwards heap requests to the original system heap.
 * @author Emery Berger, Juan Altmayer Pizzorno
 **/

#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__)

// FIXME use the non-__APPLE__ code on __APPLE__ as well to reduce unnecessary variants?
class SysMallocHeap {
 public:
  enum { Alignment = alignof(max_align_t) };

  inline void *malloc(size_t sz) {
    return ::malloc(sz);
  }

  inline void *memalign(size_t alignment, size_t size) {
    void *buf;
    if (::posix_memalign(&buf, alignment, size) != 0) {
      return nullptr;
    }
    return buf;
  }

  inline bool free(void *ptr) {
    ::free(ptr);
    return true;
  }

  inline size_t getSize(void *ptr) {
#if defined(__APPLE__)
    return ::malloc_size(ptr);
#else
    return ::malloc_usable_size(const_cast<void *>(ptr));
#endif
  }
};

#elif !defined(_WIN32) // __APPLE__

#include <dlfcn.h>

/**
 * Provides access to the original system heap, even if the standard malloc/free/...
 * have been redirected by way of LD_PRELOAD, MacOS interpose, etc.
 */
class SysMallocHeap {
  decltype(::malloc)* _malloc{0};
  decltype(::free)* _free{0};
  decltype(::memalign)* _memalign{0};
  decltype(::malloc_usable_size)* _malloc_usable_size{0};

 public:
  enum { Alignment = alignof(max_align_t) };

  SysMallocHeap() :
      _malloc((decltype(_malloc)) dlsym(RTLD_NEXT, "malloc")),
      _free((decltype(_free)) dlsym(RTLD_NEXT, "free")),
      _memalign((decltype(_memalign)) dlsym(RTLD_NEXT, "memalign")),
      _malloc_usable_size((decltype(_malloc_usable_size)) dlsym(RTLD_NEXT, "malloc_usable_size")) {}

  inline void *malloc(size_t sz) {
    return (*_malloc)(sz);
  }

  inline void *memalign(size_t alignment, size_t sz) {
    return (*_memalign)(alignment, sz);
  }

  inline void free(void *ptr) {
    (*_free)(ptr);
  }

  inline size_t getSize(void *ptr) {
    return (*_malloc_usable_size)(ptr);
  }
};

#endif

} // namespace

#endif
