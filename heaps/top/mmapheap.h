/* -*- C++ -*- */

/*

  Heap Layers: An Extensible Memory Allocation Infrastructure
  
  Copyright (C) 2000-2020 by Emery Berger
  http://www.emeryberger.com
  emery@cs.umass.edu
  
  Heap Layers is distributed under the terms of the Apache 2.0 license.

  You may obtain a copy of the License at
  http://www.apache.org/licenses/LICENSE-2.0

*/

#ifndef HL_MMAPHEAP_H
#define HL_MMAPHEAP_H

#if defined(_WIN32)
#include <windows.h>
#else
// UNIX
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <map>
#endif

#include <new>

#include "heaps/buildingblock/freelistheap.h"
#include "heaps/special/zoneheap.h"
#include "heaps/special/bumpalloc.h"
#include "heaps/threads/lockedheap.h"
#include "locks/posixlock.h"
#include "threads/cpuinfo.h"
#include "utility/myhashmap.h"
#include "wrappers/mmapwrapper.h"
#include "wrappers/stlallocator.h"

#ifndef HL_MMAP_PROTECTION_MASK
#if HL_EXECUTABLE_HEAP
#if !defined(MAP_JIT)
#define MAP_JIT 0
#endif
#define HL_MMAP_PROTECTION_MASK (PROT_READ | PROT_WRITE | PROT_EXEC | MAP_JIT)
#else
#define HL_MMAP_PROTECTION_MASK (PROT_READ | PROT_WRITE)
#endif
#endif


#if !defined(MAP_ANONYMOUS) && defined(MAP_ANON)
#define MAP_ANONYMOUS MAP_ANON
#endif


/**
 * @class MmapHeap
 * @brief A "source heap" that manages memory via calls to the VM interface.
 * @author Emery Berger
 */

namespace HL {

  /**
   * @class SizedMmapHeap
   * @brief A heap around mmap, but only a sized-free is supported for Unix-like systems.
   */
  class SizedMmapHeap {
  public:

    /// All memory from here is zeroed.
    enum { ZeroMemory = 1 };

    enum { Alignment = MmapWrapper::Alignment };

#if defined(_WIN32) 

    static inline void * malloc (size_t sz) {
#if HL_EXECUTABLE_HEAP
      char * ptr = (char *) VirtualAlloc (NULL, sz, MEM_RESERVE | MEM_COMMIT | MEM_TOP_DOWN, PAGE_EXECUTE_READWRITE);
#else
      char * ptr = (char *) VirtualAlloc (NULL, sz, MEM_RESERVE | MEM_COMMIT | MEM_TOP_DOWN, PAGE_READWRITE);
#endif
      return (void *) ptr;
    }
  
    static inline void free (void * ptr, size_t) {
      // No need to keep track of sizes in Windows.
      VirtualFree (ptr, 0, MEM_RELEASE);
    }

    static inline void free (void * ptr) {
      // No need to keep track of sizes in Windows.
      VirtualFree (ptr, 0, MEM_RELEASE);
    }
  
    inline static size_t getSize (void * ptr) {
      MEMORY_BASIC_INFORMATION mbi;
      VirtualQuery (ptr, &mbi, sizeof(mbi));
      return (size_t) mbi.RegionSize;
    }

#else

    static inline void * malloc (size_t sz) {
      // Round up to the size of a page.
      sz = (sz + CPUInfo::PageSize - 1) & (size_t) ~(CPUInfo::PageSize - 1);
      void * addr = 0;
      int flags = 0;
      static int fd = -1;
#if defined(MAP_ALIGN) && defined(MAP_ANON)
      addr = Alignment;
      flags |= MAP_PRIVATE | MAP_ALIGN | MAP_ANON;
#elif !defined(MAP_ANONYMOUS)
      if (fd == -1) {
	fd = ::open ("/dev/zero", O_RDWR);
      }
      flags |= MAP_PRIVATE;
#else
      flags |= MAP_PRIVATE | MAP_ANONYMOUS;
#endif

      auto ptr = mmap (addr, sz, HL_MMAP_PROTECTION_MASK, flags, fd, 0);
      if (ptr == MAP_FAILED) {
	ptr = nullptr;
      }
      return ptr;
    }
    
    static void free (void * ptr, size_t sz)
    {
      if ((long) sz < 0) {
	abort();
      }
      munmap (reinterpret_cast<char *>(ptr), sz);
    }

#endif

  };


  class MmapHeap : public SizedMmapHeap {
#if !defined(_WIN32)

  private:

    // (Now disabled)
    // Note: we never reclaim memory obtained for MyHeap, even when
    // this heap is destroyed.
    // class MyHeap : public LockedHeap<PosixLockType, FreelistHeap<BumpAlloc<16384, SizedMmapHeap>>> {
    class MyHeap : public LockedHeap<PosixLockType, FreelistHeap<ZoneHeap<SizedMmapHeap, 16384>>> {
    };

    typedef MyHashMap<void *, size_t, MyHeap> mapType;

  protected:
    mapType MyMap;

    PosixLockType MyMapLock;

  public:

    enum { Alignment = SizedMmapHeap::Alignment };

    inline void * malloc (size_t sz) {
      void * ptr = SizedMmapHeap::malloc (sz);
      MyMapLock.lock();
      MyMap.set (ptr, sz);
      MyMapLock.unlock();
      assert (reinterpret_cast<size_t>(ptr) % Alignment == 0);
      return const_cast<void *>(ptr);
    }

    inline size_t getSize (void * ptr) {
      MyMapLock.lock();
      size_t sz = MyMap.get (ptr);
      MyMapLock.unlock();
      return sz;
    }

#if 1
    void free (void * ptr, size_t sz) {
      SizedMmapHeap::free (ptr, sz);
    }
#endif

    inline void free (void * ptr) {
      assert (reinterpret_cast<size_t>(ptr) % Alignment == 0);
      MyMapLock.lock();
      size_t sz = MyMap.get (ptr);
      SizedMmapHeap::free (ptr, sz);
      MyMap.erase (ptr);
      MyMapLock.unlock();
    }
#endif
  };

}

#endif
