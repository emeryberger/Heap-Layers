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

#ifndef HL_HEAPREDIRECT_H
#define HL_HEAPREDIRECT_H 

#if !defined(likely)
  #define likely(x) __builtin_expect(!!(x), 1)
#endif
#if !defined(unlikely)
  #define unlikely(x) __builtin_expect(!!(x), 0)
#endif
#if !defined ATTRIBUTE_EXPORT
  #define ATTRIBUTE_EXPORT __attribute__((visibility("default")))
#endif

namespace HL {

/**
 * Redirects the system heap calls to a custom heap, using Heap-Layers' wrappers.
 *
 * In order to service ::malloc through a custom heap, the custom heap's constructor
 * and its malloc method need to run; if either one of these, directly or indirectly,
 * need to call ::malloc, that would lead to an infinite recursion.  To avoid this, we
 * detect such recursive ::malloc calls using a thread-local boolean variable and service
 * them from a statically allocated heap.
 *
 * The thread-local variable's and the static heap's initialization must be done carefully
 * so as not to require ::malloc when no heap is available to service it from. To further
 * complicate things, ::malloc is/can be invoked early during executable startup, before
 * C++ constructors for global objects.  On MacOS, this seems to happen before thread
 * initialization: attempts to use thread_local or __thread lead to an abort() in _tlv_bootstrap.
 *
 * To work around the C++ constructor issue, we use static initializers within functions;
 * for the thread_local/__thread issue, we use POSIX thread-local data.  We prefer __atomic
 * calls to std::atomic because STL code feels more likely to invoke malloc.
 * We use std::recursive_mutex and std::lock_guard to avoid introducing more code and because
 * they don't seem to require malloc, but may need to move to pthread_mutex if that should change.
 *
 * The problem is really that "won't use malloc" AFAIK isn't part of most of these primitives'
 * contracts, so we stand on shaky ground.
 *
 * @author Juan Altmayer Pizzorno
 */
template<typename CustomHeapType, int STATIC_HEAP_SIZE> 
class HeapWrapper {
  typedef LockedHeap<std::mutex, StaticBufferHeap<STATIC_HEAP_SIZE>> StaticHeapType;

// Using a static bool is not thread safe, but gives us a benchmarking
// baseline for this implementation.
#define HW_USE_STATIC_BOOL 0

  static bool* getInMallocFlag() {
#if !HW_USE_STATIC_BOOL
    // modified double-checked locking pattern (https://en.wikipedia.org/wiki/Double-checked_locking)
    static enum {NEEDS_KEY=0, CREATING_KEY=1, DONE=2} inMallocKeyState{NEEDS_KEY};
    static pthread_key_t inMallocKey;
    static std::recursive_mutex m;

    auto state = __atomic_load_n(&inMallocKeyState, __ATOMIC_ACQUIRE);
    if (state != DONE) {
      std::lock_guard<decltype(m)> g{m};

      state = __atomic_load_n(&inMallocKeyState, __ATOMIC_RELAXED);
      if (unlikely(state == CREATING_KEY)) {
        return nullptr; // recursively invoked
      }
      else if (unlikely(state == NEEDS_KEY)) {
        __atomic_store_n(&inMallocKeyState, CREATING_KEY, __ATOMIC_RELAXED);
        if (pthread_key_create(&inMallocKey, 0) != 0) { // may call malloc/calloc/...
          abort();
        }
        __atomic_store_n(&inMallocKeyState, DONE, __ATOMIC_RELEASE);
      }
    }

    bool* flag = (bool*)pthread_getspecific(inMallocKey); // not expected to call malloc
    if (unlikely(flag == nullptr)) {
      std::lock_guard<decltype(m)> g{m};

      static bool initializing = false;
      if (initializing) {
        return nullptr; // recursively invoked
      }

      initializing = true;
      flag = (bool*)getHeap<StaticHeapType>()->malloc(sizeof(bool));
      *flag = false;
      if (pthread_setspecific(inMallocKey, flag) != 0) { // may call malloc/calloc/...
        abort();
      }
      initializing = false;
    }

    return flag;
#else
    static bool inMalloc = false;
    return &inMalloc;
#endif
  }

 public:
  template<class HEAP>
  static HEAP* getHeap() {
    // Allocate heap on first use and never destroy it, for malloc and such
    // may still be used in atexit() 
    alignas(std::max_align_t) static char buffer[sizeof(HEAP)];
    static HEAP* heap = new (buffer) HEAP;
    return heap;
  }

  static void* xxmalloc(size_t sz) {
    bool* inMalloc = getInMallocFlag();
    if (unlikely(inMalloc == nullptr || *inMalloc)) {
      return getHeap<StaticHeapType>()->malloc(sz);
    }

    *inMalloc = true;
    void* ptr = getHeap<CustomHeapType>()->malloc(sz);
    *inMalloc = false;
    return ptr;
  }

  static void *xxmemalign(size_t alignment, size_t sz) {
    bool* inMalloc = getInMallocFlag();
    if (unlikely(inMalloc == nullptr || *inMalloc)) {
      return getHeap<StaticHeapType>()->memalign(alignment, sz);
    }

    *inMalloc = true;
    void* ptr = getHeap<CustomHeapType>()->memalign(alignment, sz);
    *inMalloc = false;
    return ptr;
  }

  static void xxfree(void* ptr) {
    if (likely(!getHeap<StaticHeapType>()->isValid(ptr))) {
      getHeap<CustomHeapType>()->free(ptr);
    }
  }

  static size_t xxmalloc_usable_size(void *ptr) {
    if (unlikely(getHeap<StaticHeapType>()->isValid(ptr))) {
      return getHeap<StaticHeapType>()->getSize(ptr);
    }

    return getHeap<CustomHeapType>()->getSize(ptr);
  }

  static void xxmalloc_lock() {
    getHeap<CustomHeapType>()->lock();
  }

  static void xxmalloc_unlock() {
    getHeap<CustomHeapType>()->unlock();
  }
};

} // namespace

#define HEAP_REDIRECT(CustomHeap, staticSize)\
  typedef HL::HeapWrapper<CustomHeap, staticSize> TheHeapWrapper;\
  extern "C" {\
    ATTRIBUTE_EXPORT void *xxmalloc(size_t sz) {\
      return TheHeapWrapper::xxmalloc(sz);\
    }\
    \
    ATTRIBUTE_EXPORT void xxfree(void *ptr) {\
      TheHeapWrapper::xxfree(ptr);\
    }\
    \
    ATTRIBUTE_EXPORT void *xxmemalign(size_t alignment, size_t sz) {\
      return TheHeapWrapper::xxmemalign(alignment, sz);\
    }\
    \
    ATTRIBUTE_EXPORT size_t xxmalloc_usable_size(void *ptr) {\
      return TheHeapWrapper::xxmalloc_usable_size(ptr);\
    }\
    \
    ATTRIBUTE_EXPORT void xxmalloc_lock() {\
      TheHeapWrapper::xxmalloc_lock();\
    }\
    \
    ATTRIBUTE_EXPORT void xxmalloc_unlock() {\
      TheHeapWrapper::xxmalloc_unlock();\
    }\
  }

#endif
