/* -*- C++ -*- */

/*

  Heap Layers: An Extensible Memory Allocation Infrastructure
  
  Copyright (C) 2000-2017 by Emery Berger
  http://www.emeryberger.com
  emery@cs.umass.edu
  
*/

/*
 * @file   libkingsley.cpp
 * @brief  This file replaces malloc etc. in your application.
 * @author Emery Berger <http://www.emeryberger.com>
 */

#include <stdlib.h>

volatile int anyThreadCreated = 1;

#include "heaplayers.h"


using namespace HL;

class TopHeap : public SizeHeap<UniqueHeap<ZoneHeap<MmapHeap, 65536> > > {};

// One heap shared by every thread, so it must be locked: even simple
// tools are multithreaded (e.g., uutils du and sort spawn worker threads).
//
// A pthread mutex everywhere, including macOS: the heap lock is held across
// fork() and released in the child (see xxmalloc_lock/unlock below), and an
// os_unfair_lock (MacLockType) cannot be unlocked there -- the child's thread
// is not its owner, and macOS kills the child.
typedef PosixLockType TheLockType;

class TheCustomHeapType :
  public ANSIWrapper<LockedHeap<TheLockType,
				KingsleyHeap<AdaptHeap<DLList, TopHeap>, TopHeap> > > {};

inline static TheCustomHeapType * getCustomHeap() {
  static char thBuf[sizeof(TheCustomHeapType)];
  static TheCustomHeapType * th = new (thBuf) TheCustomHeapType;
  return th;
}

#if defined(_WIN32)
#pragma warning(disable:4273)
#endif

#include "printf.h"

#if !defined(_WIN32)
#include <unistd.h>

extern "C" {
  // For use by the replacement printf routines (see
  // https://github.com/emeryberger/printf)
  void _putchar(char ch) { ::write(1, (void *)&ch, 1); }
}
#endif

#include "wrappers/generic-memalign.cpp"

extern "C" {
  
  void * xxmalloc (size_t sz) {
    // printf_("xxmalloc\n");
    auto ptr = getCustomHeap()->malloc (sz);
    // printf_("xxmalloc %lu = %p\n", sz, ptr);
    return ptr;
  }

  void xxfree (void * ptr) {
    getCustomHeap()->free (ptr);
  }

  void xxfree_sized (void * ptr, size_t) {
    getCustomHeap()->free (ptr);
  }

  void xxfree_aligned_sized (void * ptr, size_t, size_t) {
    getCustomHeap()->free (ptr);
  }

  void * xxmemalign(size_t alignment, size_t sz) {
    return generic_xxmemalign(alignment, sz);
  }
  
  size_t xxmalloc_usable_size (void * ptr) {
    return getCustomHeap()->getSize (ptr);
  }

  // Held across fork() so the child never inherits the heap lock in a
  // locked state (macwrapper.cpp calls these from the _malloc_fork_* hooks).
  void xxmalloc_lock() {
    getCustomHeap()->lock();
  }

  void xxmalloc_unlock() {
    getCustomHeap()->unlock();
  }
  
}

#if !defined(__APPLE__) && !defined(_WIN32)
#include <pthread.h>

// The GNU wrapper has no fork hooks, so register them here. This runs from
// a constructor rather than on first allocation because pthread_atfork can
// itself allocate.
__attribute__((constructor))
static void kingsleyRegisterForkHandlers() {
  pthread_atfork(xxmalloc_lock, xxmalloc_unlock, xxmalloc_unlock);
}
#endif

