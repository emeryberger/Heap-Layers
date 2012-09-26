// -*- C++ -*-

/*

  Heap Layers: An Extensible Memory Allocation Infrastructure
  
  Copyright (C) 2000-2012 by Emery Berger
  http://www.cs.umass.edu/~emery
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

#ifndef HL_KINGSLEYHEAP_H
#define HL_KINGSLEYHEAP_H

#include "heaps/combining/strictsegheap.h"

/**
 * @file kingsleyheap.h
 * @brief Classes to implement a Kingsley (power-of-two, segregated fits) allocator.
 */

/**
 * @namespace Kingsley
 * @brief Functions to implement KingsleyHeap.
 */



namespace Kingsley {

  inline size_t class2Size (const int i) {
    size_t sz = (size_t) (1 << (i+3));
    return sz;
  }

  static inline int ceilLog2 (size_t);

  inline int size2Class (const size_t sz) {
    int cl = ceilLog2((sz < 8) ? 8 : sz) - 3;
    return cl;
  }

  /// Quickly calculate the CEILING of the log (base 2) of the argument.
#if defined(_WIN32)
  static inline int ceilLog2 (size_t sz)
  {
    int retval;
    sz = (sz << 1) - 1;
    __asm {
      bsr eax, sz
        mov retval, eax
        }
    return retval;
  }
#elif defined(__GNUC__) && defined(__i386__)
  static inline int ceilLog2 (size_t sz)
  {
    sz = (sz << 1) - 1;
    asm ("bsrl %0, %0" : "=r" (sz) : "0" (sz));
    return (int) sz;
  }
#elif 0 // defined(__GNUC__) && defined(__x86_64__)
  static inline int ceilLog2 (size_t sz)
  {
    sz = (sz << 1) - 1;
    asm ("bsrq %0, %0" : "=r" (sz) : "0" (sz));
    return (int) sz;
  }
#elif defined(__GNUC__)
  // Just use the intrinsic.
  static inline int ceilLog2 (size_t sz)
  {
    sz = (sz << 1) - 1;
    return (sizeof(unsigned long) * 8) - __builtin_clzl(sz) - 1;
  }
#else
  static inline int ceilLog2 (size_t v) {
    int log = 0;
    unsigned int value = 1;
    while (value < v) {
      value <<= 1;
      log++;
    }
    return log;
  }
#endif

  enum { NUMBINS = 29 };

}

/**
 * @class KingsleyHeap
 * @brief The Kingsley-style allocator.
 * @param PerClassHeap The heap to use for each size class.
 * @param BigHeap The heap for "large" objects.
 * @see Kingsley
 */

namespace HL {

template <class PerClassHeap, class BigHeap>
  class KingsleyHeap :
   public StrictSegHeap<Kingsley::NUMBINS,
                        Kingsley::size2Class,
                        Kingsley::class2Size,
                        PerClassHeap,
                        BigHeap> {};

}

#endif
