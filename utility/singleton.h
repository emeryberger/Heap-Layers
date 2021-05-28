// -*- C++ -*-

#ifndef HL_SINGLETON_H
#define HL_SINGLETON_H

#include <new>

namespace HL {

  template <class C>
  class singleton {
  public:
    
    static C& getInstance() {
#if 0
      static C theSingleton;
      return theSingleton;
#else
      // This is used with heaps, and when a heap replaces the system heap it needs
      // to continue existing past global destructor invocation, as it may be called
      // from atexit(), etc.
      alignas(std::max_align_t) static char buf[sizeof(C)];
      static C * theSingleton = new (buf) C;
      return *theSingleton;
#endif
    }

    singleton()= default;
    ~singleton()= default;
    singleton(const singleton&)= delete;
    singleton& operator=(const singleton&)= delete;
  };

}

#endif

