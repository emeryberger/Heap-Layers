// -*- C++ -*-

#ifndef HL_BACKTRACE_H
#define HL_BACKTRACE_H

/**
 * @class BacktraceHeap
 * @brief Saves the call stack for each allocation, displaying them on demand.
 **/

#include <cxxabi.h>

#if defined(__linux__) || defined(__APPLE__)
    #include <execinfo.h>
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
      if (obj == NULL) return obj;

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
          std::cerr << "---\n";
        }
        std::cerr << SuperHeap::getSize(obj) << " byte(s) leaked @ " << obj+1 << "\n";

        char** strs = backtrace_symbols(obj->callStack, obj->nFrames);
        for (int i=0; i<obj->nFrames; i++) {
          char function[128] = "";
  
          bool understood = false;
          #if defined(__linux__)
            // "./x(_ZN2HL13BacktraceHeapINS_10MallocHeapELi32EE6mallocEm+0x54) [0x55b472bd8cba]"
            understood = (sscanf(strs[i], "%*[^(](%128[^+]+0x%*x)]", function) == 1);
          #elif defined(__APPLE__)
            // "0   x                                   0x000000010f720e70 _Z3barv + 32"
            understood = (sscanf(strs[i], "%*s %*s %*s %128s + %*d", function) == 1);
          #endif
  
          if (!understood) {
            std::cerr << indent << strs[i] << "\n";
            continue;
          }
  
          int demangleStatus = 0;
          char* cppName = abi::__cxa_demangle(function, NULL, 0, &demangleStatus);
  
          if (demangleStatus == 0) {
            std::cerr << indent << cppName << "\n";
          }
          else {
            std::cerr << indent << function << "\n";
          }

          ::free(cppName);
          any = true;
        }
        ::free(strs);
      }
    }
  };
}

#endif
