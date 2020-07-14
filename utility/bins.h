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

#ifndef HL_BINS_H
#define HL_BINS_H

#include <cstdlib>
#include <assert.h>
#include <stddef.h>

#include "ilog2.h"
//#include "unistd.h"

namespace HL {
  
  template <class Header, size_t Size>
    class bins;

  /// Default, generic, power of two bins for undefined sizes.
  template <class Header, size_t Size>
  class bins {
  public:

    bins()
    {
      static_assert(BIG_OBJECT > 0, "BIG_OBJECT must be positive.");
      static_assert(getClassSize(0) < getClassSize(1), "Need distinct size classes.");
      static_assert(getSizeClass(getClassSize(0)) == 0, "Size class computation error.");
      static_assert(getSizeClass(0) >= getSizeClass(alignof(max_align_t)), "Min size must be at least alignof(max_align_t).");
#ifndef NDEBUG
      int bins = 0;
      for (auto i = alignof(max_align_t); i < BIG_OBJECT; i++) {
	bins++;
	int sc = getSizeClass(i);
	assert (getClassSize(sc) >= i);
	assert (sc == 0 ? true : (getClassSize(sc-1) < i));
	assert (getSizeClass(getClassSize(sc)) == sc);
      }
      assert(bins == NUM_BINS);
#endif
    }

  private:

    // constexpr integer log base two calculations, ONLY for use during compilation.
    
    static constexpr inline unsigned int ilog2(const size_t n) {
      return ((n<=1) ? 0 : 1 + ilog2(n/2));
    }
    
    static constexpr inline auto ilog2_ceil (const size_t n)
    {
      return ilog2(n * 2 - 1);
    }
    
    
  public:

    enum { BIG_OBJECT = Size / 8 }; // - sizeof(Header) };
    enum { NUM_BINS   = ilog2_ceil(Size) - ilog2_ceil(alignof(max_align_t)) + 1 };
    enum { NumBins = NUM_BINS };
    enum { MaxObjectSize = BIG_OBJECT };
    enum { LogMaxAlignT = ilog2(alignof(max_align_t)) };
    
    static inline constexpr int getSizeClass (size_t sz) {
      sz = (sz < alignof(max_align_t)) ? alignof(max_align_t) : sz;
      auto sizeClass = (int) HL::ilog2(sz) - LogMaxAlignT; // (int) HL::ilog2(alignof(max_align_t));
      return sizeClass;
    }

    static constexpr inline size_t getClassSize(int i) {
      return (alignof(max_align_t) << i);
    }
    
    static constexpr inline size_t getClassMaxSize(int i) {
      return getClassSize(i);
    }
    
  };
  
}


#endif
