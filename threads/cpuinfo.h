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

#ifndef HL_CPUINFO_H
#define HL_CPUINFO_H

#include "utility/arch.h"

#if defined(_WIN32)
#include <windows.h>
#include <process.h>
#else
#include <unistd.h>
#endif


#if !defined(_WIN32)
#include <pthread.h>
#endif

#if defined(__SVR4) // Solaris
#include <sys/lwp.h>
extern "C" unsigned int lwp_self(void);
#include <thread.h>
extern "C" int _thr_self(void);
#endif

#if defined(__linux)
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#endif

#if defined(__APPLE__)
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

#if defined(__sgi)
#include <sys/types.h>
#include <sys/sysmp.h>
#include <sys/sysinfo.h>
#endif

#if defined(hpux)
#include <sys/mpctl.h>
#endif

#if defined(_WIN32)
extern __declspec(thread) int localThreadId;
#endif

#if defined(__SVR4) && defined(MAP_ALIGN)
extern volatile int anyThreadStackCreated;
#endif

namespace HL {

/**
 * @class CPUInfo
 * @author Emery Berger <http://www.cs.umass.edu/~emery>
 *
 * @brief Architecture-independent wrapper to get number of CPUs. 
 */

class CPUInfo {
public:

  #ifdef HL_APPLE_SILICON
  enum { PageSize = 16384UL };
  #else
  // Good for practically all platforms.
  enum { PageSize = 4096UL };
  #endif

  inline static int getNumProcessors() {
    static int _numProcessors = computeNumProcessors();
    return _numProcessors;
  }

  static inline unsigned int getThreadId();
  inline static int computeNumProcessors();

};


int CPUInfo::computeNumProcessors (void)
{
  static int np = 0;
  if (!np) {
#if defined(_SC_NPROCESSORS_ONLN)
    np = (int) sysconf(_SC_NPROCESSORS_ONLN);
#elif defined(_WIN32)
    SYSTEM_INFO infoReturn[1];
    GetSystemInfo (infoReturn);
    np = (int) (infoReturn->dwNumberOfProcessors);
#elif defined(__sgi)
    np = (int) sysmp(MP_NAPROCS);
#elif defined(hpux)
    np = mpctl(MPC_GETNUMSPUS, NULL, NULL); // or pthread_num_processors_np()?
#else
    np = 2;
    // Unsupported platform.
    // Pretend we have at least two processors. This approach avoids the risk of assuming
    // we're on a uniprocessor, which might lead clever allocators to avoid using atomic
    // operations for all locks.
#endif
    return np;
  } else {
    return np;
  }
}

#if defined(USE_THREAD_KEYWORD)
  extern __thread int localThreadId;
#endif

unsigned int CPUInfo::getThreadId() {
#if defined(__SVR4)
  return (unsigned int) pthread_self();
#elif defined(_WIN32)
  // It looks like thread id's are always multiples of 4, so...
  return GetCurrentThreadId() >> 2;
#elif defined(__APPLE__)
  // FIXME: is this page size dependent?
  // Consecutive thread id's in Mac OS are 4096 apart;
  // dividing off the 4096 gives us an appropriate thread id.
  unsigned int tid = (unsigned int) (((size_t) pthread_self()) >> 12);
  return tid;
#elif defined(__BEOS__)
  return find_thread(0);
#elif defined(__linux)
  return (unsigned int) ((size_t) pthread_self()) >> 12;
  //  return (unsigned int) syscall (SYS_gettid);
#elif defined(PTHREAD_KEYS_MAX)
  // As with Apple, above.
  return (unsigned int) ((size_t) pthread_self()) >> 12;
#elif defined(POSIX)
  return (unsigned int) pthread_self();
#elif USE_SPROC
  // This hairiness has the same effect as calling getpid(),
  // but it's MUCH faster since it avoids making a system call
  // and just accesses the sproc-local data directly.
  unsigned int pid = (unsigned int) PRDA->sys_prda.prda_sys.t_pid;
  return pid;
#else
#error "This platform is not currently supported."
  return 0;
#endif
}

}

#endif
