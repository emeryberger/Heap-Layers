/* -*- C++ -*- */

/*

  Heap Layers: An Extensible Memory Allocation Infrastructure
  
  Copyright (C) 2000-2003 by Emery Berger
  http://www.cs.umass.edu/~emery
  emery@cs.umass.edu
  
*/

/*
 * @file   libreap.cpp
 * @brief  Replaces malloc and adds reap functionality to your application.
 * @author Emery Berger <http://www.cs.umass.edu/~emery>
 */

#include <stdlib.h>
#include <new>

#include "heaplayers.h"
#include "slopheap.h"
#include "regionheap.h"
#include "chunkheap.h"
#include "oneheap.h"
#include "regionheapapi.h" 
#include "nestedheap.h"


// Conservative assumption here...
// Note: this is used by wrapper.cpp.
volatile int anyThreadCreated = 1;

// All reaps eventually come from mmap.
class OneStore : public MmapHeap {};

// We'll grab chunks of 8K.
class TopHeap :
  public ChunkHeap<8192 - 20, SlopHeap<RegionHeap<OneStore>, 16> > {
};

#define MAIN_ALLOCATOR TopHeap

class TheCustomHeapType : public Reap<MAIN_ALLOCATOR> {};

inline static TheCustomHeapType * getCustomHeap (void) {
  static char thBuf[sizeof(TheCustomHeapType)];
  static TheCustomHeapType * th = new (thBuf) TheCustomHeapType;
  return th;
}

#include "wrapper.cpp"

extern "C" void regionCreate (void ** reg, void ** parent)
{
  Reap<MAIN_ALLOCATOR> * psr;
  psr = new Reap<MAIN_ALLOCATOR> ();
  if (parent) {
    (*((Reap<MAIN_ALLOCATOR> **) parent))->addChild (psr);
  }

  *((Reap<MAIN_ALLOCATOR> **) reg) = psr;
}


extern "C" void regionDestroy (void ** reg)
{
  delete ((Reap<MAIN_ALLOCATOR> *) *reg);
  *reg = NULL;
}

extern "C" void * regionAllocate (void ** reg, size_t sz)
{
  void * ptr = ((Reap<MAIN_ALLOCATOR> *) *reg)->malloc (sz);
  return ptr;
}

extern "C" void regionFreeAll (void ** reg)
{
  ((Reap<MAIN_ALLOCATOR> *) *reg)->clear ();
}

extern "C" void regionFree (void ** reg, void * ptr)
{
  ((Reap<MAIN_ALLOCATOR> *) *reg)->free (ptr);
}

// Reap API wrappers.


extern "C" void reapcreate (void ** reg, void ** parent)
{
  regionCreate (reg, parent);
}

extern "C" void reapdestroy (void ** reg)
{
  regionDestroy (reg);
}

extern "C" void * reapmalloc (void ** reg, size_t sz)
{
  return regionAllocate (reg, sz);
}

extern "C" void reapclear (void ** reg)
{
  regionFreeAll (reg);
}

extern "C" void reapfree (void ** reg, void * ptr)
{
  regionFree (reg, ptr);
}

