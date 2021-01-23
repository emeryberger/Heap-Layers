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

/**
 * @file strictsegheap.h
 * @brief Definition of StrictSegHeap.
 */

#ifndef HL_STRICTSEGHEAP_H
#define HL_STRICTSEGHEAP_H

#include "segheap.h"

/**
 * @class StrictSegHeap
 * @brief A "strict" segregated-fits collection of (homogeneous) heaps.
 *
 * One extra heap is used for objects that are "too big".  Unlike
 * SegHeap, StrictSegHeap does not perform splitting to satisfy memory
 * requests. If no memory is available from the appropriate bin,
 * malloc returns NULL.
 *
 * @sa SegHeap
 *
 * @param NumBins The number of bins (subheaps).
 * @param getSizeClass Function to compute size class from size.
 * @param getClassMaxSize Function to compute the largest size for a given size class.
 * @param LittleHeap The subheap class.
 * @param BigHeap The parent class, used for "big" objects.
 */

namespace HL {

  template <int NumBins,
	    int (*size2class) (const size_t),
	    size_t (*class2size) (const int),
	    class LittleHeap,
	    class BigHeap>
  class StrictSegHeap :
    public SegHeap<NumBins, size2class, class2size, LittleHeap, BigHeap>
  {
  private:

    typedef SegHeap<NumBins, size2class, class2size, LittleHeap, BigHeap> SuperHeap;

  public:

    void clear () {
      for (auto i = 0; i < NumBins; i++) {
        const size_t sz = class2size(i);
        void * ptr;
        while ((ptr = SuperHeap::myLittleHeap[i].malloc (sz)) != NULL) {
          SuperHeap::bigheap.free (ptr);
        }
      }
      for (auto j = 0; j < SuperHeap::NUM_ULONGS; j++) {
        SuperHeap::binmap[j] = 0;
      }
      SuperHeap::_memoryHeld = 0;
    }

    /**
     * Malloc from exactly one available size.
     * (don't look in every small heap, as in SegHeap).
     */

    inline void * malloc (const size_t sz) {
      void * ptr = nullptr;
      const auto sizeClass   = size2class(sz);
      const auto realSize = class2size(sizeClass);

      assert (realSize >= sz);

      if (realSize <= SuperHeap::_maxObjectSize) {
        assert (sizeClass >= 0);
        assert (sizeClass < NumBins);
        ptr = SuperHeap::myLittleHeap[sizeClass].malloc (realSize);
      }
      if (!ptr) {
        ptr = SuperHeap::bigheap.malloc (realSize);
      }
      return ptr;
    }

    inline void free (void * ptr) {
      const auto objectSize = SuperHeap::getSize(ptr);
      free(ptr, objectSize);
    }

    inline void free(void * ptr, size_t objectSize) {
      if (objectSize > SuperHeap::_maxObjectSize) {
        SuperHeap::bigheap.free (ptr);
      } else {
        auto objectSizeClass = size2class(objectSize);
        assert (objectSizeClass >= 0);
        assert (objectSizeClass < NumBins);

        // Put the freed object into the right sizeclass heap.

        // Ensure that the bin that we are going to put it in is for
        // objects that are no bigger than the actual size of the
        // object.
        while (class2size(objectSizeClass) > objectSize)
          objectSizeClass--;

        SuperHeap::myLittleHeap[objectSizeClass].free (ptr);
      }
    }

  };

}

#endif

