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

#ifndef HL_MALLOCINFO_H
#define HL_MALLOCINFO_H

#include <limits.h>

namespace HL {
  
  class MallocInfo {
  public:
    // Prevent integer overflows by restricting allocation size (usually 2GB).
    enum { MaxSize = UINT_MAX / 2 };

#if defined(__LP64__) || defined(_LP64) || defined(__APPLE__) || defined(_WIN64) || defined(__x86_64__)
    enum { MinSize = 16UL };
    enum { Alignment = 16UL };
#else
    enum { MinSize   = sizeof(double) };
    enum { Alignment = sizeof(double) };
#endif
  };

}

#endif
