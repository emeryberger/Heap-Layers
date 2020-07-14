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

#ifndef HL_SLLIST_H_
#define HL_SLLIST_H_

#include <assert.h>

/**
 * @class SLList
 * @brief A "memory neutral" (intrusive) singly-linked list.
 * @author Emery Berger
 */

namespace HL {

  class SLList {
  public:
    
    inline SLList() {
      clear();
    }

    class Entry;
  
    /// Clear the list.
    inline void clear() {
      head.next = nullptr;
    }

    /// Is the list empty?
    inline bool isEmpty() const {
      return (head.next == nullptr);
    }

    /// Get the head of the list.
    inline Entry * get() {
      const Entry * e = head.next;
      if (e == nullptr) {
	return nullptr;
      }
      head.next = e->next;
      return (Entry *) e;
    }

  private:

    /**
     * @brief Remove one item from the list.
     * @warning This method aborts the program if called.
     */
    inline void remove (Entry *) {
      abort();
    }

  public:

    /// Inserts an entry into the head of the list.
    inline void insert (void * ePtr) {
      Entry * e = (Entry *) ePtr;
      e->next = head.next;
      head.next = e;
    }

    /// An entry in the list.
    class Entry {
    public:
      inline Entry()
	: next (nullptr)
      {}
      //  private:
      //    Entry * prev;
    public:
      Entry * next;
    };

  private:

    /// The head of the list.
    Entry head;

  };

}

#endif
