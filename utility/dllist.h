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

#ifndef HL_DLLIST_H
#define HL_DLLIST_H

#include <assert.h>

/**
 *
 * @class DLList
 * @brief A "memory neutral" (intrusive) doubly-linked list.
 * @author Emery Berger
 */

namespace HL {

class DLList {
public:

  inline DLList() {
    clear();
  }

  class Entry;
  
  /// Clear the list.
  inline void clear() {
    head.setPrev (&head);
    head.setNext (&head);
  }

  /// Is the list empty?
  inline bool isEmpty() const {
    return (head.getNext() == &head);
  }

  /// Get the head of the list.
  inline Entry * get() {
    const Entry * e = head.next;
    if (e == &head) {
      return nullptr;
    }
    head.next = e->next;
    head.next->prev = &head;
    return (Entry *) e;
  }

  /// Remove one item from the list.
  inline void remove (Entry * e) {
    e->remove();
  }

  /// Insert an entry into the head of the list.
  inline void insert (Entry * e) {
    e->insert (&head, head.next);
  }

  /// An entry in the list.
  class Entry {
  public:
    //  private:
    inline void setPrev (Entry * p) { assert (p != nullptr); prev = p; }
    inline void setNext (Entry * p) { assert (p != nullptr); next = p; }
    inline Entry * getPrev() const { return prev; }
    inline Entry * getNext() const { return next; }
    inline void remove() const {
      prev->setNext(next);
      next->setPrev(prev);
    }
    inline void insert (Entry * p, Entry * n) {
      prev = p;
      next = n;
      p->setNext (this);
      n->setPrev (this);
    }
    Entry * prev;
    Entry * next;
  };


private:

  /// The head of the list.
  Entry head;

};

}

#endif
