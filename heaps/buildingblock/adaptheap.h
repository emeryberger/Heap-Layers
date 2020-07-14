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


#ifndef HL_ADAPTHEAP_H
#define HL_ADAPTHEAP_H

#include <assert.h>
#include <stdlib.h>

/**
 * @class AdaptHeap
 * @brief Maintains dictionary entries through freed objects.
 * Sample dictionaries include DLList and SLList.
 */

namespace HL {

  template <class Dictionary, class SuperHeap>
  class AdaptHeap : public SuperHeap {
  public:

    enum { Alignment = SuperHeap::Alignment };

    /// Allocate an object (remove from the dictionary).
    inline void * malloc (const size_t) {
      void * ptr = (Entry *) dict.get();
      if (ptr) {
        assert (SuperHeap::getSize(ptr) >= sizeof(dict));
      }
      return ptr;
    }

    /// Deallocate the object (return to the dictionary).
    inline void free (void * ptr) {
      if (ptr) {
        assert (SuperHeap::getSize(ptr) >= sizeof(dict));
        Entry * entry = (Entry *) ptr;
        dict.insert (entry);
      }
    }

    /// Remove an object from the dictionary.
    inline int remove (void * ptr) {
      if (ptr) {
        assert (SuperHeap::getSize(ptr) >= sizeof(dict));
        dict.remove ((Entry *) ptr);
      }
      return 1;
    }

    /// Clear the dictionary.
    inline void clear (void) {
      Entry * ptr;
      while ((ptr = (Entry *) dict.get()) != NULL) {
        SuperHeap::free (ptr);
      }
      dict.clear();
      SuperHeap::clear();
    }


  private:

    /// The dictionary object.
    Dictionary dict;

    class Entry : public Dictionary::Entry {};
  };

}

#endif // HL_ADAPTHEAP
