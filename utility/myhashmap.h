// -*- C++ -*-

#ifndef HL_MYHASHMAP_H
#define HL_MYHASHMAP_H

/*

  Heap Layers: An Extensible Memory Allocation Infrastructure
  
  Copyright (C) 2000-2020 by Emery Berger
  http://www.emeryberger.com
  emery@cs.umass.edu
  
  Heap Layers is distributed under the terms of the Apache 2.0 license.

  You may obtain a copy of the License at
  http://www.apache.org/licenses/LICENSE-2.0

*/

#include <assert.h>
#include <new>

#include "hash.h"

namespace HL {

  template <typename Key,
	    typename Value,
	    class Allocator>
  class MyHashMap {

  public:

    MyHashMap (unsigned int size = INITIAL_NUM_BINS)
      : _numBins (size)
    {
      assert (_numBins > 0);
      void * buf = _allocator.malloc (sizeof(ListNodePtr) * _numBins);
      assert(buf);
      if (buf == nullptr) {
	abort();
      }
      _bins = new (buf) ListNodePtr[_numBins];
      for (unsigned int i = 0 ; i < _numBins; i++) {
	_bins[i] = NULL;
      }
    }

    void set (Key k, Value v) {
      unsigned int binIndex = (unsigned int) (Hash<Key>::hash(k) % _numBins);
      ListNode * l = _bins[binIndex];
      while (l != NULL) {
	if (l->key == k) {
	  l->value = v;
	  return;
	}
	l = l->next;
      }
      // Didn't find it.
      insert (k, v);
    }

    Value get (Key k) {
      unsigned int binIndex = (unsigned int) (Hash<Key>::hash(k) % _numBins);
      ListNode * l = _bins[binIndex];
      while (l != NULL) {
	if (l->key == k) {
	  return l->value;
	}
	l = l->next;
      }
      // Didn't find it.
      return 0;
    }

    void erase (Key k) {
      unsigned int binIndex = (unsigned int) (Hash<Key>::hash(k) % _numBins);
      ListNode * curr = _bins[binIndex];
      ListNode * prev = NULL;
      while (curr != NULL) {
	if (curr->key == k) {
	  // Found it.
	  if (curr != _bins[binIndex]) {
	    assert (prev->next == curr);
	    prev->next = prev->next->next;
	    _allocator.free (curr);
	  } else {
	    ListNode * n = _bins[binIndex]->next;
	    _allocator.free (_bins[binIndex]);
	    _bins[binIndex] = n;
	  }
	  return;
	}
	prev = curr;
	curr = curr->next;
      }
    }


  private:

    void insert (Key k, Value v) {
      unsigned int binIndex = (unsigned int) (Hash<Key>::hash(k) % _numBins);
      void * ptr = _allocator.malloc (sizeof(ListNode));
      if (ptr) {
	ListNode * l = new (ptr) ListNode;
	l->key = k;
	l->value = v;
	l->next = _bins[binIndex];
	_bins[binIndex] = l;
      }
    }

    enum { INITIAL_NUM_BINS = 511 };

    class ListNode {
    public:
      ListNode (void)
	: next (NULL)
      {}
      Key key;
      Value value;
      ListNode * next;
    };

    unsigned long 	_numBins;

    typedef ListNode * 	ListNodePtr;
    ListNodePtr * 	_bins;
    Allocator 		_allocator;
  };

}

#endif
