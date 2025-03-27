/* -*- C++ -*- */

/*

  Heap Layers: An Extensible Memory Allocation Infrastructure
  
  Copyright (C) 2000-2017 by Emery Berger
  http://www.emeryberger.com
  emery@cs.umass.edu
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

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

class TheCustomHeapType :
  public ANSIWrapper<KingsleyHeap<AdaptHeap<DLList, TopHeap>, TopHeap> > {};

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
    auto ptr = getCustomHeap()->malloc (sz);
    // printf_("xxmalloc %lu = %p\n", sz, ptr);
    return ptr;
  }

  void xxfree (void * ptr) {
    getCustomHeap()->free (ptr);
  }

  void * xxmemalign(size_t alignment, size_t sz) {
    return generic_xxmemalign(alignment, sz);
  }
  
  size_t xxmalloc_usable_size (void * ptr) {
    return getCustomHeap()->getSize (ptr);
  }

  void xxmalloc_lock() {
    // getCustomHeap()->lock();
  }

  void xxmalloc_unlock() {
    // getCustomHeap()->unlock();
  }
  
}

