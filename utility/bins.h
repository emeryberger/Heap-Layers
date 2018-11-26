/*

  Heap Layers: An Extensible Memory Allocation Infrastructure
  
  Copyright (C) 2000-2018 by Emery Berger
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

#pragma once

//#include "tprintf.hh"

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
      static_assert(getSizeClass(0) >= getSizeClass(sizeof(max_align_t)), "Min size must be at least max_align_t.");
      static_assert(getSizeClass(0) >= getSizeClass(alignof(max_align_t)), "Min size must be at least alignof(max_align_t).");
#ifndef NDEBUG
      int bins = 0;
      for (auto i = sizeof(max_align_t); i < BIG_OBJECT; i++) {
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
    
    static constexpr inline unsigned int ilog2_ceil (const size_t n)
    {
      return ilog2(n * 2 - 1);
    }
    
    
  public:

    enum { BIG_OBJECT = Size / 8 }; // - sizeof(Header) };
    enum { NUM_BINS   = ilog2_ceil(Size) - ilog2_ceil(sizeof(max_align_t)) + 1 };

    static inline constexpr int getSizeClass (size_t sz) {
      sz = (sz < sizeof(max_align_t)) ? sizeof(max_align_t) : sz;
      return (int) HL::ilog2(sz) - (int) HL::ilog2(sizeof(max_align_t));
    }

    static constexpr inline size_t getClassSize (int i) {
      return (sizeof(max_align_t) << i);
    }
    
  };
  
}


#endif
