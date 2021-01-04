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
      static char buf[sizeof(C)];
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

