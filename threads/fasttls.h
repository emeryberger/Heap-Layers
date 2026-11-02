/* -*- C++ -*- */

#ifndef HL_FASTTLS_H
#define HL_FASTTLS_H

/*

  Heap Layers: An Extensible Memory Allocation Infrastructure

  Copyright (C) 2000-2026 by Emery Berger
  http://www.emeryberger.com
  emery@cs.umass.edu

  Heap Layers is distributed under the terms of the Apache 2.0 license.

  You may obtain a copy of the License at
  http://www.apache.org/licenses/LICENSE-2.0

*/

/**
 * @file   fasttls.h
 * @brief  A pthread key whose get() is an inline thread-pointer read on Darwin.
 *
 * On macOS, __thread / thread_local always go through _tlv_get_addr (the
 * initial-exec TLS model does not help), and pthread_getspecific is an
 * out-of-line call into libsystem_pthread. Both are expensive on an
 * allocator fast path.
 *
 * On Darwin, a pthread_key_t is a direct index into the calling thread's
 * TSD array, whose base is the thread pointer: tpidrro_el0 (with the low
 * 3 bits masked) on arm64, %gs on x86_64. So pthread_getspecific(key) is
 * exactly tsd[key], and we can read it inline in ~3 instructions. This is
 * the technique mimalloc uses, except mimalloc (and Hoard) hardcode a
 * reserved slot; using the key pthread_key_create hands us cannot collide
 * with another allocator in the same process.
 *
 * The TSD layout is an implementation detail, so create() verifies it
 * (write via pthread_setspecific, read back directly) and silently falls
 * back to pthread_getspecific if it does not hold. Writes always go through
 * pthread_setspecific so libpthread's bookkeeping (and key destructors)
 * behave normally; only reads are direct.
 *
 * ARM64 note: it must be tpidrro_el0, NOT tpidr_el0; the latter (and
 * __builtin_thread_pointer()) reads the wrong register on macOS.
 *
 * On other platforms (or with HL_DISABLE_FAST_TSD) FastThreadKey is a thin
 * pthread_getspecific wrapper.
 */

#if !defined(_WIN32)

#include <pthread.h>
#include <stddef.h>
#include <stdint.h>
#include <atomic>

// Define HL_DISABLE_FAST_TSD to force the portable pthread_getspecific path.
#if defined(__APPLE__) && (defined(__aarch64__) || defined(__x86_64__)) \
  && !defined(HL_DISABLE_FAST_TSD)
#define HL_FAST_TSD 1
#else
#define HL_FAST_TSD 0
#endif

namespace HL {

  class FastThreadKey {
  public:

    // constexpr so that a static FastThreadKey is constant-initialized
    // (zerofill, no dynamic initializer, no guard variable). That matters
    // for allocators, which are called before static constructors run.
    constexpr FastThreadKey()
      : _slot (0),
        _created (false),
        _key ()
    {}

    /// Create the underlying key. Not thread-safe: call under pthread_once.
    bool create(void (*destructor)(void *)) {
      if (pthread_key_create(&_key, destructor) != 0) {
        return false;
      }
#if HL_FAST_TSD
      // Verify that the key really indexes the TSD array before enabling
      // direct reads. Keys are small (dynamic ones start at 256 and the
      // array holds 768), so the probe read itself stays in bounds.
      const size_t k = static_cast<size_t>(_key);
      if (k != 0 && k < MaxDirectKey) {
        int probe = 0;
        bool ok = (pthread_setspecific(_key, &probe) == 0)
          && (readSlot(k) == &probe);
        pthread_setspecific(_key, nullptr);
        ok = ok && (readSlot(k) == nullptr);
        if (ok) {
          _slot.store(k, std::memory_order_relaxed);
        }
      }
#endif
      _created.store(true, std::memory_order_release);
      return true;
    }

    /// True once create() has completed (acquire: _key is then visible).
    bool isCreated() const {
      return _created.load(std::memory_order_acquire);
    }

    /// True if get() is the inline direct read.
    bool isDirect() const {
      return _slot.load(std::memory_order_relaxed) != 0;
    }

    /// The value for this thread. Must not be called before create().
    inline void * get() const {
#if HL_FAST_TSD
      // One load of _slot (it holds the key itself, so no ordering with
      // any other field is needed), then the thread-pointer read.
      const size_t k = _slot.load(std::memory_order_relaxed);
      if (__builtin_expect(k != 0, 1)) {
        return readSlot(k);
      }
#endif
      return pthread_getspecific(_key);
    }

    /// Like get(), but returns nullptr (rather than reading an unrelated
    /// slot) if direct access is not enabled yet; lets callers keep a
    /// single fast-path branch and do creation in their slow path.
    inline void * getIfDirect() const {
#if HL_FAST_TSD
      const size_t k = _slot.load(std::memory_order_relaxed);
      if (__builtin_expect(k != 0, 1)) {
        return readSlot(k);
      }
#endif
      return nullptr;
    }

    inline int set(const void * value) {
      return pthread_setspecific(_key, value);
    }

    pthread_key_t key() const {
      return _key;
    }

  private:

#if HL_FAST_TSD
    enum { MaxDirectKey = 768 };

    /// tsd[k] for the calling thread.
    static inline void * readSlot(size_t k) {
      void * v;
#if defined(__aarch64__)
      void ** tsd;
      __asm__ volatile ("mrs %0, tpidrro_el0\n\tbic %0, %0, #7" : "=r" (tsd));
      v = tsd[k];
#else // __x86_64__: %gs is based at the TSD array
      __asm__ volatile ("movq %%gs:(,%1,8), %0" : "=r" (v) : "r" (k));
#endif
      return v;
    }
#endif

    std::atomic<size_t> _slot;   // key if direct reads are verified, else 0
    std::atomic<bool>   _created;
    pthread_key_t       _key;
  };

}

#endif // !_WIN32

#endif
