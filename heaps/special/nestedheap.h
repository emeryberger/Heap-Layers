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

#ifndef HL_NESTEDHEAP_H_
#define HL_NESTEDHEAP_H_

#include <assert.h>

/**
 * @class NestedHeap
 * @brief Hierarchical heaps.
 * @author Emery Berger <http://www.cs.umass.edu/~emery>
 */

namespace HL {

template <class SuperHeap>
class NestedHeap : public SuperHeap {
public:

  NestedHeap()
    : parent (NULL),
      child (NULL),
      prev (NULL),
      next (NULL)
  {
  }

  ~NestedHeap()
  {
    clear();
    if (parent != NULL) {
      parent->removeChild (this);
    }
    removeSibling (this);
  }

  inline void clear() {

    // Clear this heap.
    SuperHeap::clear();

#if 0
    //
    // Iterate through all children and delete them.
    //

    if (child != NULL) {
      NestedHeap<SuperHeap> * nextChild = child->next;
      while (child != NULL) {
        NestedHeap<SuperHeap> * prevChild = child->prev;
        delete child;
        child = prevChild;
      }
      child = nextChild;
      while (child != NULL) {
        nextChild = child->next;
        delete child;
        child = nextChild;
      }
    }
    assert (child == NULL);

#else // clear all the children.

    NestedHeap<SuperHeap> * ch = child;
    while (ch != NULL) {
      NestedHeap<SuperHeap> * nextChild = ch->next;
      ch->clear();
      ch = nextChild;
    }
#endif

  }

  void addChild (NestedHeap<SuperHeap> * ch)
  {
    if (child == NULL) {
      child = ch;
      child->prev = NULL;
      child->next = NULL;
    } else {
      assert (child->prev == NULL);
      assert (ch->next == NULL);
      ch->prev = NULL;
      ch->next = child;
      child->prev = ch;
      child = ch;
    }
    child->parent = this;
  }

private:

  void removeChild (NestedHeap<SuperHeap> * ch)
  {
    assert (ch != NULL);
    if (child == ch) {
      if (ch->prev) {
        child = ch->prev;
      } else if (ch->next) {
        child = ch->next;
      } else {
        child = NULL;
      }
    }
    removeSibling (ch);
  }

  inline static void removeSibling (NestedHeap<SuperHeap> * sib)
  {
    if (sib->prev) {
      sib->prev->next = sib->next;
    }
    if (sib->next) {
      sib->next->prev = sib->prev;
    }
  }

  NestedHeap<SuperHeap> * parent;
  NestedHeap<SuperHeap> * child;
  NestedHeap<SuperHeap> * prev;
  NestedHeap<SuperHeap> * next;

};

}

#endif
