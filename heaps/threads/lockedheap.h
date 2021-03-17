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

#ifndef HL_LOCKEDHEAP_H
#define HL_LOCKEDHEAP_H

#include <mutex>
#include <cstddef>

namespace HL {

  template <class LockType, class Super>
  class LockedHeap : public Super {
  public:

    enum { Alignment = Super::Alignment };

    inline void * malloc (size_t sz) {
      std::lock_guard<LockType> l (thelock);
      return Super::malloc (sz);
    }

    inline auto free (void * ptr) {
      std::lock_guard<LockType> l (thelock);
      return Super::free (ptr);
    }

    inline auto free (void * ptr, size_t sz) {
      std::lock_guard<LockType> l (thelock);
      return Super::free (ptr, sz);
    }

    inline void * memalign (size_t alignment, size_t sz) {
      std::lock_guard<LockType> l (thelock);
      return Super::memalign (alignment, sz);
    }

    inline size_t getSize (void * ptr) const {
      std::lock_guard<LockType> l (thelock);
      return Super::getSize (ptr);
    }

    inline size_t getSize (void * ptr) {
      std::lock_guard<LockType> l (thelock);
      return Super::getSize (ptr);
    }

    inline void lock() {
      thelock.lock();
    }

    inline void unlock() {
      thelock.unlock();
    }

  private:
    //    char dummy[128]; // an effort to avoid false sharing.
    LockType thelock;
  };

}

#endif
