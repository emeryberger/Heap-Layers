// -*- C++ -*-

#ifndef HL_BACKTRACE_H
#define HL_BACKTRACE_H

/**
 * @class BacktraceHeap
 * @brief Saves the call stack for each allocation, displaying them on demand.
 **/

#include <cxxabi.h>
#include <iomanip>


#if defined(__linux__) || defined(__APPLE__)
    #include <execinfo.h>
    #include <dlfcn.h>
#else
    #error "needs port"
#endif

namespace HL {

  template <class SuperHeap, int stackSize = 16>
  class BacktraceHeap : public SuperHeap {
    struct TraceObj {
      int nFrames;
      void* callStack[stackSize];
      TraceObj* next;
      TraceObj* prev;
    };

    // Recursive mutex because our malloc() and free() may be invoked from within print_leaks()
    // if ::malloc() and ::free() point to this heap by way of LD_PRELOAD or some such mechanism.
    std::recursive_mutex _mutex;
    TraceObj* _objects;

    void link(TraceObj* obj) {
      std::lock_guard<std::recursive_mutex> guard(_mutex);

      obj->prev = 0;
      obj->next = _objects;
      if (_objects) _objects->prev = obj;
      _objects = obj;
    }

    void unlink(TraceObj* obj) {
      std::lock_guard<std::recursive_mutex> guard(_mutex);

      if (_objects == obj) _objects = obj->next;
      if (obj->prev) obj->prev->next = obj->next;
      if (obj->next) obj->next->prev = obj->prev;
    }

  public:

    void* malloc(size_t sz) {
      TraceObj* obj = (TraceObj*) SuperHeap::malloc(sz + sizeof(TraceObj));
      if (obj == 0) return obj;

      obj->nFrames = backtrace(obj->callStack, stackSize);
      link(obj);

      return obj + 1;
    }


    void free(void* ptr) {
      TraceObj* obj = (TraceObj*)ptr;
      --obj;

      unlink(obj);
      SuperHeap::free(obj);
    }


    void print_leaks() {
      static const std::string indent = "  ";

      // Note that our malloc() and free() may be invoked recursively from here
      // if ::malloc() and ::free() point to this heap by way of LD_PRELOAD or
      // some such mechanism.
      std::lock_guard<std::recursive_mutex> guard(_mutex);

      bool any = false;

      for (auto obj = _objects; obj; obj = obj->next) {
        if (any) {
          std::cerr << "---\n";     // XXX does cerr output use malloc?
        }
        std::cerr << SuperHeap::getSize(obj) << " byte(s) leaked @ " << obj+1 << "\n";

        for (int i=0; i<obj->nFrames; i++) {
          std::cerr << indent << std::setw(2+2*8) // "0x" + 64-bit ptr
                              << obj->callStack[i];

          Dl_info info;
          if (dladdr(obj->callStack[i], &info) && info.dli_sname != 0) {
            // XXX now looking for non-malloc __cxa_demangle
            int demangleStatus = 0;
            char* cppName = abi::__cxa_demangle(info.dli_sname, 0, 0, &demangleStatus);
    
            if (demangleStatus == 0) {
              std::cerr << " " << cppName;
            }
            else {
              std::cerr << " " << info.dli_sname;
            }
            ::free(cppName);

            typedef unsigned long long int ull;
            std::cerr << " + " << (ull)obj->callStack[i] - (ull)info.dli_saddr;
          }

          std::cerr << "\n";
        }
        any = true;
      }
    }
  };
}

#endif
