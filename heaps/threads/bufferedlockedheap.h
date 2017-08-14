#ifndef HL_BUFFEREDLOCKEDHEAP_H
#define HL_BUFFEREDLOCKEDHEAP_H

#include <mutex>
#include <cstddef>

namespace HL {

  template <int Size, class LockType, class Super>
  class BufferedLockedHeap : public Super {
  public:

    enum { Alignment = Super::Alignment };

    void * malloc (size_t sz) {
      std::lock_guard<LockType> l (thelock);
      void *ptr = Super::malloc (sz);
      if (ptr != NULL) {
        return ptr;
      }
      // Emptying the freeBuffer and trying again
      emptyFreeBuffer();
      return Super::malloc (sz);
    }

    void free (void *ptr) {
      if (freeIndex == Size) {
        std::lock_guard<LockType> l (thelock);
        emptyFreeBuffer();
      }
      if (freeBuffer == NULL) {
        init();
      }
      freeBuffer[freeIndex] = ptr;
      freeIndex++;
    }

    inline size_t getSize (void * ptr) const {
      std::lock_guard<LockType> l (thelock);
      return Super::getSize (ptr);
    }

    inline size_t getSize (void * ptr) {
      std::lock_guard<LockType> l (thelock);
      return Super::getSize (ptr);
    }

    inline void lock (void) {
      thelock.lock();
    }

    inline void unlock (void) {
      thelock.unlock();
    }

    void clear (void) {
      std::lock_guard<LockType> l (thelock);
      emptyFreeBuffer();
      Super::free (freeBuffer);
    }

  private:

    void init (void) {
      std::lock_guard<LockType> l (thelock);
      freeBuffer = reinterpret_cast<void **>
        (Super::malloc (sizeof(void *) * Size));
      freeIndex = 0;
    }

    void emptyFreeBuffer (void) {
      for (int i = 0; i < freeIndex; i++) {
        Super::free (freeBuffer[i]);
      }
      freeIndex = 0;
    }

    static __thread void **freeBuffer;
    static __thread int freeIndex;

    LockType thelock;
  };

  template <int Size, class LockType, class Super>
  __thread void **BufferedLockedHeap<Size, LockType, Super>::freeBuffer = NULL;
  template <int Size, class LockType, class Super>
  __thread int BufferedLockedHeap<Size, LockType, Super>::freeIndex = 0;

}

#endif
