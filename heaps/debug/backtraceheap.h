// -*- C++ -*-

#ifndef HL_BACKTRACE_H
#define HL_BACKTRACE_H

/**
 * @class BacktraceHeap
 * @brief Saves the call stack for each allocation
 *
 **/

#include <cxxabi.h>

#if defined(__linux__) || defined(__APPLE__)
    #include <execinfo.h>
#else
    #error "needs port"
#endif

namespace HL {

  template <class SuperHeap, int stackSize = 32>
  class BacktraceHeap : public SuperHeap {
    struct TraceObj {
      int nFrames;
      void* callStack[stackSize];
    };

  public:
    void* malloc(size_t sz) {
      TraceObj* ptr = (TraceObj*) SuperHeap::malloc(sz + sizeof(TraceObj));
      if (ptr == NULL) return ptr;

      ptr->nFrames = backtrace(ptr->callStack, stackSize);
      return ptr + 1;
    }

    void free(void* ptr) {
      TraceObj* obj = (TraceObj*)ptr;
      SuperHeap::free(obj - 1);
    }

    std::string trace(void* ptr) {
      TraceObj* obj = (TraceObj*)ptr;
      --obj;

      char** strs = backtrace_symbols(obj->callStack, obj->nFrames);
      string ret;
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
            ret.append(strs[i]).append("\n");
            continue;
        }

        int demangleStatus = 0;
        char* cppName = abi::__cxa_demangle(function, NULL, 0, &demangleStatus);

        if (demangleStatus == 0) {
          ret.append(cppName);
        }
        else {
          ret.append(function);
        }
        ::free(cppName);

        ret.append("\n");
      }
      ::free(strs);
      return ret;
    }
  };
}

#endif
