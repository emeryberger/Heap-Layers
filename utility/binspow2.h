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

#pragma once

#if !defined(HL_BINSPOW2_H)
#define HL_BINSPOW2_H

#include <cstdlib>
#include <assert.h>
#include <stddef.h>

#include "bins.h"
#include "ilog2.h"

namespace HL {

  template <size_t MaxSize>
  class bins_pow2 {
  public:

    bins_pow2()
    {
      static_assert(BIG_OBJECT > 0, "BIG_OBJECT must be positive.");
      static_assert(getClassSize(0) < getClassSize(1), "Need distinct size classes.");
      static_assert(getSizeClass(getClassSize(0)) == 0, "Size class computation error.");
      static_assert(getSizeClass(0) >= getSizeClass(sizeof(max_align_t)), "Min size must be at least max_align_t.");
      static_assert(getSizeClass(0) >= getSizeClass(alignof(max_align_t)), "Min size must be at least alignof(max_align_t).");
#ifndef NDEBUG
      for (unsigned long i = sizeof(max_align_t); i < BIG_OBJECT; i++) {
	int sc = getSizeClass(i);
	assert (getClassSize(sc) >= i);
	assert (sc == 0 ? true : (getClassSize(sc-1) < i));
	assert (getSizeClass(getClassSize(sc)) == sc);
      }
#endif
    }

  private:

    static constexpr inline unsigned int ilog2_constexpr (size_t v) {
      unsigned int log = 0;
      unsigned int value = 1;
      while (value < v) {
	value <<= 1;
	log++;
      }
      return log;
    }
    
  public:

    enum { BIG_OBJECT = MaxSize };
    enum { MaxObjectSize = BIG_OBJECT };
    enum { NUM_BINS   = HL::ilog2(MaxSize) - HL::ilog2(sizeof(max_align_t)) + 1 };
    enum { NumBins    = NUM_BINS };

    static inline constexpr int getSizeClass (size_t sz) {
      sz = (sz < sizeof(max_align_t)) ? sizeof(max_align_t) : sz;
      return static_cast<int>(HL::ilog2(sz) - HL::ilog2(sizeof(max_align_t)));
    }

    static constexpr inline size_t getClassSize (int i) {
      return (sizeof(max_align_t) << i);
    }

    static constexpr inline size_t getClassMaxSize(int i) {
      return getClassSize(i);
    }
  };

}



#endif
