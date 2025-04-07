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

#ifndef HL_MMAPWRAPPER_H
#define HL_MMAPWRAPPER_H

#include "utility/arch.h"

#if defined(_WIN32)
#include <windows.h>
#else
// UNIX
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#if defined(__linux__)
#include <sys/prctl.h>
#if !defined(PR_SET_VMA)
#define PR_SET_VMA 0x53564d41
#define PR_SET_VMA_ANON_NAME 0
#endif
#endif
#include <map>
#endif

#if defined(__SVR4)
// Not sure why this is needed for Solaris, but there it is.
extern "C" int madvise (caddr_t, size_t, int);
#endif

#if !defined(HL_MMAP_PROTECTION_MASK)
#if HL_EXECUTABLE_HEAP
#define HL_MMAP_PROTECTION_MASK (PROT_READ | PROT_WRITE | PROT_EXEC)
#else
#if !defined(PROT_MAX)
#define PROT_MAX(p) 0
#endif
#define HL_MMAP_PROTECTION_MASK (PROT_READ | PROT_WRITE | PROT_MAX(PROT_READ | PROT_WRITE))
#endif
#endif

#if !defined(MAP_ANONYMOUS) && defined(MAP_ANON)
#define MAP_ANONYMOUS MAP_ANON
#endif

namespace HL {

  class MmapWrapper {
  public:

#if defined(_WIN32) 
  
    // Microsoft Windows has 4K pages aligned to a 64K boundary.
    enum { Size = 4 * 1024UL };
    enum { Alignment = 64 * 1024UL };

#elif defined(__SVR4)
    // Solaris aligns 8K pages to a 64K boundary.
    enum { Size = 8 * 1024UL };
    enum { Alignment = 64 * 1024UL };

#elif defined(HL_APPLE_SILICON)
    // macOS on Apple Silicon aligns 16K pages to a 16K boundary.
    enum { Size = 16 * 1024UL };
    enum { Alignment = 16 * 1024UL };

#else
    // Linux and most other operating systems align memory to a 4K boundary.
    enum { Size = 4 * 1024UL };
    enum { Alignment = 4 * 1024UL };

#endif


    // Release the given range of memory to the OS (without unmapping it).
    void release (void * ptr, size_t sz) {
      if ((size_t) ptr % Alignment == 0) {
	// Extra sanity check in case the superheap's declared alignment is wrong!
#if defined(_WIN32)
	VirtualAlloc (ptr, sz, MEM_RESET, PAGE_NOACCESS);
#elif defined(__APPLE__)
	madvise (ptr, sz, MADV_DONTNEED);
	madvise (ptr, sz, MADV_FREE);
#else
	// Assume Unix platform.
	madvise ((caddr_t) ptr, sz, MADV_DONTNEED);
#endif
      }
    }

#if defined(_WIN32) 
  
    static void protect (void * ptr, size_t sz) {
      DWORD oldProtection;
      VirtualProtect (ptr, sz, PAGE_NOACCESS, &oldProtection);
    }
    
    static void unprotect (void * ptr, size_t sz) {
      DWORD oldProtection;
      VirtualProtect (ptr, sz, PAGE_READWRITE, &oldProtection);
    }

    static void * map (size_t sz) {
      void * ptr;
#if HL_EXECUTABLE_HEAP
      const int permflags = PAGE_EXECUTE_READWRITE;
#else
      const int permflags = PAGE_READWRITE;
#endif
      ptr = VirtualAlloc(nullptr, sz, MEM_RESERVE | MEM_COMMIT | MEM_TOP_DOWN, permflags);
      return  ptr;
    }
  
    static void unmap (void * ptr, size_t) {
      VirtualFree (ptr, 0, MEM_RELEASE);
    }

#else // UNIX

    static void protect (void * ptr, size_t sz) {
      mprotect ((char *) ptr, sz, PROT_NONE);
    }
    
    static void unprotect (void * ptr, size_t sz) {
      mprotect ((char *) ptr, sz, PROT_READ | PROT_WRITE | PROT_EXEC);
    }

    static void * map (size_t sz) {

      if (sz == 0) {
	return nullptr;
      }

      // Round up the size to a page-sized value.
      sz = Size * ((sz + Size - 1) / Size);

      void * ptr;
      int mapFlag = 0;
      char * startAddress = 0;

#if defined(MAP_ALIGN) && defined(MAP_ANON)
      int fd = -1;
      startAddress = (char *) Alignment;
      mapFlag |= MAP_PRIVATE | MAP_ALIGN | MAP_ANON;
#elif defined(MAP_ALIGNED)
      int fd = -1;
      // On allocations equal or larger than page size, we align it to the log2 boundary
      // in those contexts, sometimes (on NetBSD notably) large mappings tends to fail
      // without this flag.
      size_t alignment = ilog2(sz);
      mapFlag |= MAP_PRIVATE | MAP_ANON;
      if (alignment >= 12ul)
          mapFlag |= MAP_ALIGNED(alignment);
#elif !defined(MAP_ANONYMOUS)
      static int fd = ::open ("/dev/zero", O_RDWR);
      mapFlag |= MAP_PRIVATE;
#else
      int fd = -1;
      //      mapFlag |= MAP_ANONYMOUS | MAP_PRIVATE;
      mapFlag |= MAP_ANON | MAP_PRIVATE;
#if HL_EXECUTABLE_HEAP
#if defined(MAP_JIT)
      mapFlag |= MAP_JIT;
#endif
#endif
#endif

      ptr = mmap(startAddress, sz, HL_MMAP_PROTECTION_MASK, mapFlag, fd, 0);
      #ifdef HL_APPLE_SILICON
        #include <mach/vm_page_size.h>
        assert(sz % vm_page_size == 0);
        assert((reinterpret_cast<uintptr_t>(ptr) % vm_page_size) == 0);
      #endif

      if (ptr == MAP_FAILED) {
	// tprintf::tprintf("MmapWrapper: MAP_FAILED\n");
	return nullptr;
      } else {
#if HL_NAMED_HEAP
        // For every anonymous map (assuming CONFIG_ANON_VMA_NAME is on), the range appears in /proc/<pid>/maps as
	// `560215e4f000-560215e81000 rw-p 00000000 00:00 0 7ffaf89c0000-7ffaf8aa1000 rw-p 00000000 00:00 0 [anon:Heap Layers]`
	//
    // By default the region name is `Heap Layers` but can be defined in heaplayers.h via the HL_HEAP_NAME constant.
    // If the kernel did not optin the `CONFIG_ANON_VMA_NAME` option, or even with old kernels prior to 5.17
    // the following call will be a no-op
        (void)prctl(PR_SET_VMA, PR_SET_VMA_ANON_NAME,
			reinterpret_cast<uintptr_t>(ptr),
			sz,
			reinterpret_cast<uintptr_t>(HL_HEAP_NAME));
#endif
	return ptr;
      }
    }

    static void unmap (void * ptr, size_t sz) {
      // Round up the size to a page-sized value.
      sz = Size * ((sz + Size - 1) / Size);
      munmap ((caddr_t) ptr, sz);
    }
   
#endif

  };

}

#endif
