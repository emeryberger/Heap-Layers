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

    static char* skipToken(char* s, int count) {
      while (count-- > 0) {
        s += strspn(s, " ");
        s += strcspn(s, " ");
      }

      s += strspn(s, " ");
      return s;
    }

    static char* tryDemangle(char* pFunction) {
        char function[64];
        int offset;

        // "symbolName + offset" expected
        sscanf(pFunction, "%64s %*s %d", function, &offset);
        int gotCppName = 0;

        char* cppName = abi::__cxa_demangle(function, NULL, 0, &gotCppName);
        return gotCppName == 0 ? cppName : 0;
    }

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
        // lines are formatted like:
        // 0   x                                   0x000000010f720e70 _Z3barv + 32
        // 1   x                                   0x000000010f7211e9 _Z3foov + 9
        char* pFunction = skipToken(strs[i], 3);

        ret.append(strs[i], pFunction);

        if (char* cppName = tryDemangle(pFunction)) {
          ret.append(cppName);
          ::free(cppName);
        }
        else {
          ret.append(pFunction);
        }

        ret.append("\n");
      }
      ::free(strs);
      return ret;
    }
  };
}

#endif
