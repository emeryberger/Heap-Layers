// -*- C++ -*-

#ifndef HL_FREESLLIST_H_
#define HL_FREESLLIST_H_

#include <assert.h>

/**
 * @class FreeSLList
 * @brief A "memory neutral" singly-linked list,
 *
 * Uses the free space in objects to store
 * the pointers.
 */


class FreeSLList {
public:

  inline void clear() {
    head.next = nullptr;
  }

  class Entry;
  
  /// Get the head of the list.
  inline Entry * get() {
    const Entry * e = head.next;
    if (e == nullptr) {
      return nullptr;
    }
    head.next = e->next;
    return const_cast<Entry *>(e);
  }

  inline Entry * remove() {
    const Entry * e = head.next;
    if (e == nullptr) {
      return nullptr;
    }
    head.next = e->next;
    return const_cast<Entry *>(e);
  }
  
  inline void insert (void * e) {
    Entry * entry = reinterpret_cast<Entry *>(e);
    entry->next = head.next;
    head.next = entry;
  }

  class Entry {
  public:
    Entry()
      : next (nullptr)
    {}
    Entry * next;
  private:
    Entry (const Entry&);
    Entry& operator=(const Entry&);
  };
  
private:
  Entry head;
};


#endif




