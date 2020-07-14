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

#ifndef HL_DYNARRAY_H
#define HL_DYNARRAY_H

#include <assert.h>
#include <cstdlib>

/**
 * @class DynamicArray
 * @brief A dynamic array that grows to fit any index for assignment.
 *
 * This array also features a clear() method,
 * to free the entire array, and a trim(n) method,
 * which tells the array it is no bigger than n elements.
 */

namespace HL {

  template <class ObjType>
    class DynamicArray {
  public:
    DynamicArray()
      : internalArray (NULL),
      internalArrayLength (0)
	{}

    ~DynamicArray()
      {
	clear();
      }

    /// Clear deletes everything in the array.
    inline void clear() {
      if (internalArray != NULL) {
	delete internalArray;
	internalArray = NULL;
	internalArrayLength = 0;
	//printf ("\ninternalArrayLength %x = %d\n", this, internalArrayLength);
      }
    }

    /// Read-only access to an array element; asserts that index is in range.
    inline const ObjType& operator[] (int index) const {
      assert (index < internalArrayLength);
      assert (index >= 0);
      return internalArray[index];
    }

    /// Access an array index by reference, growing the array if necessary.
    inline ObjType& operator[] (int index) {
      assert (index >= 0);
      if (index >= internalArrayLength) {

	// This index is beyond the current size of the array.
	// Grow the array by doubling and copying the old array into the new.

	const int newSize = index * 2 + 1;
	ObjType * arr = new ObjType[newSize];
#if MALLOC_TRACE
	printf ("m %x %d\n", arr, newSize * sizeof(ObjType));
#endif
	if (internalArray != NULL) {
	  memcpy (arr, internalArray, internalArrayLength * sizeof(ObjType));
	  delete internalArray;
#if MALLOC_TRACE
	  printf ("f %x\n", internalArray);
#endif
	}
	internalArray = arr;
	internalArrayLength = newSize;
	//printf ("\ninternalArrayLength %x = %d\n", this, internalArrayLength);
      }
      return internalArray[index];
    }

    /**
     * Trim informs the array that it is now only nelts long
     * as far as the client is concerned. This may trigger
     * shrinking of the array.
     */
    inline void trim (int nelts) {

      // Halve the array if the number of elements
      // drops below one-fourth of the array size.

      if (internalArray != NULL) {
	if (nelts * 4 < internalArrayLength) {
	  const int newSize = nelts * 2;
	  ObjType * arr = new ObjType[newSize];
#if MALLOC_TRACE
	  printf ("m %x %d\n", arr, newSize * sizeof(ObjType));
#endif
	  memcpy (arr, internalArray, sizeof(ObjType) * nelts);
	  delete internalArray;
#if MALLOC_TRACE
	  printf ("f %x\n", internalArray);
#endif
	  internalArray = arr;
	  internalArrayLength = newSize;
	}
	assert (nelts <= internalArrayLength);
      }
    }


  private:

    /// The pointer to the current array.
    ObjType * internalArray;

    /// The length of the internal array, in elements.
    int internalArrayLength;
  };

}

#endif
