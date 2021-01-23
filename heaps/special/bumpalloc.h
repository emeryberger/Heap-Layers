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

#ifndef HL_BUMPALLOC_H
#define HL_BUMPALLOC_H

#include <cstddef>

#include "utility/gcd.h"

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#endif

/**
 * @class BumpAlloc
 * @brief Obtains memory in chunks and bumps a pointer through the chunks.
 * @author Emery Berger <http://www.cs.umass.edu/~emery>
 */

namespace HL {

  template <size_t ChunkSize,
	    class SuperHeap,
	    size_t Alignment_ = 1UL>
  class BumpAlloc : public SuperHeap {
  public:

    enum { Alignment = Alignment_ };

    BumpAlloc()
      : _bump (nullptr),
	_remaining (0)
    {
      static_assert((int) gcd<ChunkSize, Alignment>::VALUE == Alignment,
		    "Alignment must be satisfiable.");
      static_assert((int) gcd<SuperHeap::Alignment, Alignment>::VALUE == Alignment,
		    "Alignment must be compatible with the SuperHeap's alignment.");
      static_assert((Alignment & (Alignment-1)) == 0,
		    "Alignment must be a power of two.");
    }

    inline void * malloc (size_t sz) {
      // Round up the size if necessary.
      size_t newSize = (sz + Alignment - 1UL) & ~(Alignment - 1UL);

      // If there's not enough space left to fulfill this request, get
      // another chunk.
      if (_remaining < newSize) {
      	refill(newSize);
      }
      // Bump that pointer.
      char * old = _bump;
      _bump += newSize;
      _remaining -= newSize;

      assert ((size_t) old % Alignment == 0);
      return old;
    }

    /// Free is disabled (we only bump, never reclaim).
    inline bool free (void *) { return false; }

  private:

    /// The bump pointer.
    char * _bump;

    /// How much space remains in the current chunk.
    size_t _remaining;

    // Get another chunk.
    void refill (size_t sz) {
      if (sz < ChunkSize) {
      	sz = ChunkSize;
      }
      _bump = (char *) SuperHeap::malloc (sz);
      assert ((size_t) _bump % Alignment == 0);
      _remaining = sz;
    }

  };

  // We are going to rename this layer to BumpHeap and deprecate BumpAlloc eventually.
  template <size_t Size, class X>
  using BumpHeap = BumpAlloc<Size, X>;
  
}

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

#endif
