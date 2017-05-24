#ifndef HL_BUFFEREDLOCKEDHEAP_H
#define HL_BUFFEREDLOCKEDHEAP_H

#include <cstddef>
#include "utility/guard.h"

namespace HL {

  template <int Size, class LockType, class Super>
  class BufferedLockedHeap : public Super {
  public:

    enum { Alignment = Super::Alignment };

    void * malloc (size_t sz) {
      Guard<LockType> l (theLock);
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
        Guard<LockType> l (theLock);
        emptyFreeBuffer();
      }
      if (freeBuffer == NULL) {
        init();
      }
      freeBuffer[freeIndex] = ptr;
      freeIndex++;
    }

    inline size_t getSize (void * ptr) const {
      Guard<LockType> l (theLock);
      return Super::getSize (ptr);
    }

    inline size_t getSize (void * ptr) {
      Guard<LockType> l (theLock);
      return Super::getSize (ptr);
    }

    inline void lock (void) {
      theLock.lock();
    }

    inline void unlock (void) {
      theLock.unlock();
    }

    void clear (void) {
      Guard<LockType> l (theLock);
      emptyFreeBuffer();
      Super::free (freeBuffer);
    }

  private:

    void init (void) {
      Guard<LockType> l (theLock);
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

    LockType theLock;
  };

  template <int Size, class LockType, class Super>
  __thread void **BufferedLockedHeap<Size, LockType, Super>::freeBuffer = NULL;
  template <int Size, class LockType, class Super>
  __thread int BufferedLockedHeap<Size, LockType, Super>::freeIndex = 0;

}

#endif
