// -*- C++ -*-

/*

  Heap Layers: An Extensible Memory Allocation Infrastructure
  
  Copyright (C) 2000-2020 by Emery Berger
  http://www.emeryberger.com
  emery@cs.umass.edu
  
  Heap Layers is distributed under the terms of the Apache 2.0 license.

  You may obtain a copy of the License at
  http://www.apache.org/licenses/LICENSE-2.0

*/

#ifndef HL_ILOG2_H
#define HL_ILOG2_H

#if defined(_WIN32)
#include <windows.h>
#endif

namespace HL {

  /// Quickly calculate the CEILING of the log (base 2) of the argument.
#if defined(_WIN32)
  static inline unsigned int ilog2 (size_t sz)
  {
    DWORD index;
    _BitScanReverse (&index, sz);
    if (!(sz & (sz-1))) {
      return index;
    } else {
      return index+1;
    }
  }
#elif defined(__GNUC__)
  // Just use the intrinsic.
  static constexpr inline unsigned int ilog2 (const size_t sz)
  {
    return ((unsigned int) (sizeof(size_t) * 8UL) - (unsigned int) __builtin_clzl((sz << 1) - 1UL) - 1);
  }
#else
#error "Could not find an efficient implementation of log2."
  static constexpr inline unsigned int ilog2 (size_t v) {
    int log = 0;
    unsigned int value = 1;
    while (value < v) {
      value <<= 1;
      log++;
    }
    return log;
  }
#endif


}

#endif
