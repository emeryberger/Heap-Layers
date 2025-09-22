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

/*
 * @file   wrapper.cpp
 * @brief  Replaces malloc with appropriate calls to TheCustomHeapType.
 * @author Emery Berger <http://www.emeryberger.com>
 */

#include "threads/cpuinfo.h"

#include <string.h> // for memcpy and memset
#include <stdlib.h> // size_t
#include <stdint.h>
#include <new>


#ifndef ATTRIBUTE_EXPORT
#define ATTRIBUTE_EXPORT
#endif

extern "C" {

  void * xxmalloc (size_t);
  void   xxfree (void *);
  void * xxmemalign(size_t, size_t);
  #if HL_USE_XXREALLOC
  void * xxrealloc(void *, size_t);
  #endif
  // Takes a pointer and returns how much space it holds.
  size_t xxmalloc_usable_size (void *);

  // Locks the heap(s), used prior to any invocation of fork().
  void xxmalloc_lock();

  // Unlocks the heap(s), after fork().
  void xxmalloc_unlock();

}

#if defined(__APPLE__)
#include <malloc/malloc.h>
#elif defined(__FreeBSD__) || defined(__NetBSD__)
#include <stdlib.h>
#else
#include <malloc.h> // for memalign
#endif

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
// Disable warnings about long (> 255 chars) identifiers.
#pragma warning(disable:4786)
// Set inlining to the maximum possible depth.
#pragma inline_depth(255)
#pragma warning(disable: 4074)	// initializers put in compiler reserved area

#pragma comment(linker, "/disallowlib:libc.lib")
#pragma comment(linker, "/disallowlib:libcd.lib")
#pragma comment(linker, "/disallowlib:libcmt.lib")
#pragma comment(linker, "/disallowlib:libcmtd.lib")
#pragma comment(linker, "/disallowlib:msvcrtd.lib")

#else
#include <errno.h>
#endif

#ifndef CUSTOM_PREFIX
#define CUSTOM_PREFIX(x) x
#endif

#define CUSTOM_MALLOC(x)     CUSTOM_PREFIX(malloc)(x)
#define CUSTOM_FREE(x)       CUSTOM_PREFIX(free)(x)
#define CUSTOM_CFREE(x)      CUSTOM_PREFIX(cfree)(x)
#define CUSTOM_REALLOC(x,y)  CUSTOM_PREFIX(realloc)(x,y)
#define CUSTOM_REALLOCARRAY(x,y,z)  CUSTOM_PREFIX(reallocarray)(x,y,z)
#define CUSTOM_CALLOC(x,y)   CUSTOM_PREFIX(calloc)(x,y)
#define CUSTOM_MEMALIGN(x,y) CUSTOM_PREFIX(memalign)(x,y)
#define CUSTOM_POSIX_MEMALIGN(x,y,z) CUSTOM_PREFIX(posix_memalign)(x,y,z)
#define CUSTOM_ALIGNED_ALLOC(x,y) CUSTOM_PREFIX(aligned_alloc)(x,y)
#define CUSTOM_GETSIZE(x)    CUSTOM_PREFIX(malloc_usable_size)(x)
#define CUSTOM_GOODSIZE(x)    CUSTOM_PREFIX(malloc_good_size)(x)
#define CUSTOM_VALLOC(x)     CUSTOM_PREFIX(valloc)(x)
#define CUSTOM_PVALLOC(x)    CUSTOM_PREFIX(pvalloc)(x)
#define CUSTOM_RECALLOC(x,y,z)   CUSTOM_PREFIX(recalloc)(x,y,z)
#define CUSTOM_STRNDUP(s,sz) CUSTOM_PREFIX(strndup)(s,sz)
#define CUSTOM_STRDUP(s)     CUSTOM_PREFIX(strdup)(s)
#define CUSTOM_GETCWD(b,s)   CUSTOM_PREFIX(getcwd)(b,s)
#define CUSTOM_GETENV(s)     CUSTOM_PREFIX(getenv)(s)

// GNU-related routines:
#define CUSTOM_MALLOPT(x,y)         CUSTOM_PREFIX(mallopt)(x,y)
#define CUSTOM_MALLOC_TRIM(s)       CUSTOM_PREFIX(malloc_trim)(s)
#define CUSTOM_MALLOC_STATS(a)      CUSTOM_PREFIX(malloc_stats)(a)
#define CUSTOM_MALLOC_GET_STATE(p)  CUSTOM_PREFIX(malloc_get_state)(p)
#define CUSTOM_MALLOC_SET_STATE(p)  CUSTOM_PREFIX(malloc_set_state)(p)
#define CUSTOM_MALLINFO(a)          CUSTOM_PREFIX(mallinfo)(a)

#if defined(_WIN32)
#define MYCDECL __cdecl
#if !defined(NO_INLINE)
#define NO_INLINE __declspec(noinline)
#endif // !defined(NO_INLINE)
#pragma inline_depth(255)

#if !defined(NDEBUG)
#define __forceinline inline
#endif // !defined(NDEBUG)

#else
#define MYCDECL
#endif // defined(_WIN32)

// #if !defined(__GNUG__) && (defined(__linux__) || defined(__APPLE__))
#if defined(NDEBUG) && ((defined(__linux__) || defined(__APPLE__)))
#define HEAP_LAYERS_INLINE __attribute__((always_inline))
#else
#define HEAP_LAYERS_INLINE
#endif // defined(__linux__) || defined(__APPLE__)

/***** generic malloc functions *****/

#include <stdio.h>

#ifndef FLATTEN
#if 1
#define FLATTEN __attribute__((flatten))
#else
#define FLATTEN
#endif
#endif

#if !defined(_WIN32)
#include <dlfcn.h>
#include <limits.h>

#if !defined(RTLD_NEXT)
#define RTLD_NEXT ((void *) -1)
#endif

extern "C" void MYCDECL CUSTOM_FREE(void *);
extern "C" void * MYCDECL CUSTOM_MALLOC(size_t) __attribute__((alloc_size(1))) __attribute((malloc));
extern "C" void * MYCDECL CUSTOM_CALLOC(size_t nelem, size_t elsize) __attribute__((alloc_size(1,2)))  __attribute((malloc));

extern "C" FLATTEN void MYCDECL CUSTOM_FREE (void * ptr)
{
  if (ptr) {
    xxfree (ptr);
  }
}

extern "C" FLATTEN void * MYCDECL CUSTOM_MALLOC(size_t sz)
{
  void * ptr = xxmalloc(sz);
  return ptr;
}

#include <atomic>

static __thread int in_dlsym { 0 };

// Wrapper around dlsym that we use in calloc, below.
extern "C" __attribute__((noinline)) void * my_dlsym(void * handle, const char * symbol) {
  ++in_dlsym;
  auto ptr = dlsym(handle, symbol);
  --in_dlsym;
  return ptr;
}

extern "C" FLATTEN void * MYCDECL CUSTOM_CALLOC(size_t nelem, size_t elsize) 
{
  // Reject calls from dlsym so it uses its own internal buffer.
  if (in_dlsym) {
    return nullptr;
  }

  size_t n = nelem * elsize;
  
  if (elsize && (nelem != n / elsize)) {
    return nullptr;
  }
  
  void * ptr = xxmalloc(n);

  // Zero out the malloc'd block.
  if (ptr) {
    memset (ptr, 0, n);
  }
  return ptr;
}


#if !defined(_WIN32)
extern "C" void * MYCDECL CUSTOM_MEMALIGN (size_t alignment, size_t size)
#if !defined(__FreeBSD__) && !defined(__NetBSD__) && !defined(__SVR4)
  throw()
#endif
  __attribute__((alloc_size(2))) __attribute__((alloc_align(1))) 
;

extern "C" FLATTEN int CUSTOM_POSIX_MEMALIGN (void **memptr, size_t alignment, size_t size)
#if !defined(__FreeBSD__) && !defined(__NetBSD__) && !defined(__SVR4)
throw()
#endif
{
  *memptr = nullptr;
  if (alignment == 0 ||
      (alignment % sizeof(void*)) != 0 ||
      (alignment & (alignment - 1)) != 0)
    {
      return EINVAL;
    }
  void * ptr = CUSTOM_MEMALIGN (alignment, size);
  if (!ptr) {
    return ENOMEM;
  } else {
    *memptr = ptr;
    return 0;
  }
}
#endif


extern "C" FLATTEN void * MYCDECL CUSTOM_MEMALIGN(size_t alignment, size_t size)
#if !defined(__FreeBSD__) && !defined(__NetBSD__) && !defined(__SVR4)
  throw()
#endif
{
  return xxmemalign(alignment, size);
}

extern "C" FLATTEN void * MYCDECL CUSTOM_ALIGNED_ALLOC(size_t alignment, size_t size)
#if !defined(__FreeBSD__) && !defined(__NetBSD__)
  throw()
#endif
{
  // Per the man page: "The function aligned_alloc() is the same as
  // memalign(), except for the added restriction that size should be
  // a multiple of alignment."
  if (alignment == 0 || (size % alignment) != 0) {
    // C11: return NULL on violation (errno is unspecified).
    return nullptr;
  }
  return CUSTOM_MEMALIGN(alignment, size);
}

extern "C" FLATTEN size_t MYCDECL CUSTOM_GETSIZE (void * ptr)
{
  return xxmalloc_usable_size (ptr);
}

extern "C" void MYCDECL CUSTOM_CFREE (void * ptr)
{
  if (ptr) {
    xxfree(ptr);
  }
}

extern "C" size_t MYCDECL CUSTOM_GOODSIZE (size_t sz) {
  return sz ? sz : 1;
}

extern "C" void * MYCDECL CUSTOM_REALLOC (void * ptr, size_t sz)
{
#if HL_USE_XXREALLOC
  void* buf = xxrealloc(ptr, sz);
  return buf;
#else
  if (!ptr) {
    ptr = xxmalloc (sz);
    return ptr;
  }
  if (sz == 0) {
    CUSTOM_FREE (ptr);
#if defined(__APPLE__)
    // 0 size = free. We return a small object.  This behavior is
    // apparently required under Mac OS X and optional under POSIX.
    return xxmalloc(1);
#else
    // For POSIX, don't return anything.
    return nullptr;
#endif
  }

  size_t objSize = CUSTOM_GETSIZE(ptr);

  void * buf = xxmalloc(sz);

  if (!buf) {
    // Leave the original ptr intact.
    return nullptr;
  }
  
  if (objSize == CUSTOM_GETSIZE(buf)) {
    // The objects are the same actual size.
    // Free the new object and return the original.
    CUSTOM_FREE(buf);
    return ptr;
  }
  // Copy the contents of the original object
  // up to the size of the new block.
  size_t minSize = (objSize < sz) ? objSize : sz;
  memcpy(buf, ptr, minSize);

  // Free the old block.
  CUSTOM_FREE(ptr);

  // Return a pointer to the new one.
  return buf;
#endif
}

extern "C" void * MYCDECL CUSTOM_REALLOCARRAY (void * ptr, size_t sz1, size_t sz2)
{
  if (sz2 != 0 && sz1 > SIZE_MAX / sz2) { // overflow
    errno = ENOMEM;
    return nullptr;
  }
  return CUSTOM_REALLOC(ptr, sz1 * sz2);
}

#if defined(__linux__)

extern "C" char * MYCDECL CUSTOM_STRNDUP(const char * s, size_t sz)
{
  char * newString = nullptr;
  if (s) {
    size_t cappedLength = strnlen (s, sz);
    if ((newString = (char *) xxmalloc(cappedLength + 1))) {
      strncpy(newString, s, cappedLength);
      newString[cappedLength] = '\0';
    }
  }
  return newString;
}
#endif

extern "C" char * MYCDECL CUSTOM_STRDUP(const char * s)
{
  char * newString = nullptr;
  if (s) {
    if ((newString = (char *) xxmalloc(strlen(s) + 1))) {
      strcpy(newString, s);
    }
  }
  return newString;
}

typedef char * getcwdFunction (char *, size_t);


extern "C"  char * MYCDECL CUSTOM_GETCWD(char * buf, size_t size)
{
  static getcwdFunction * real_getcwd
    = reinterpret_cast<getcwdFunction *>
    (reinterpret_cast<uintptr_t>(my_dlsym (RTLD_NEXT, "getcwd")));

  if (!buf) {
    if (size == 0) {
      size = PATH_MAX;
    }
    buf = (char *) xxmalloc(size);
  }
  return (real_getcwd)(buf, size);
}

#endif


extern "C" int  CUSTOM_MALLOPT (int /* param */, int /* value */) {
  // NOP.
  return 1; // success.
}

extern "C" int xxmalloc_TRIM(size_t /* pad */) {
  // NOP.
  return 0; // no memory returned to OS.
}

extern "C" void xxmalloc_STATS() {
  // NOP.
}

extern "C" void * xxmalloc_GET_STATE() {
  return nullptr; // always returns "error".
}

extern "C" int xxmalloc_SET_STATE(void * /* ptr */) {
  return 0; // success.
}

// NOTE-- MUSL defines GNUC for some reason but does not have mallinfo
#if defined(__GNUC__) && defined(__GLIBC__) && !defined(__FreeBSD__) && !defined(__NetBSD__)
extern "C" struct mallinfo CUSTOM_MALLINFO() {
  // For now, we return useless stats.
  struct mallinfo m;
  m.arena = 0;
  m.ordblks = 0;
  m.smblks = 0;
  m.hblks = 0;
  m.hblkhd = 0;
  m.usmblks = 0;
  m.fsmblks = 0;
  m.uordblks = 0;
  m.fordblks = 0;
  m.keepcost = 0;
  return m;
}
#endif

#if defined(__SVR4)
// Apparently we no longer need to replace new and friends for Solaris.
#define NEW_INCLUDED
#endif


#ifndef NEW_INCLUDED
#define NEW_INCLUDED

ATTRIBUTE_EXPORT void * FLATTEN operator new (size_t sz)
#if defined(_GLIBCXX_THROW)
  _GLIBCXX_THROW (std::bad_alloc)
#endif
{
  void * ptr = xxmalloc (sz);
  if (ptr) {
    return ptr;
  }
  throw std::bad_alloc();
}

ATTRIBUTE_EXPORT void FLATTEN operator delete (void * ptr)
#if !defined(linux_)
  throw ()
#endif
{
  CUSTOM_FREE (ptr);
}

#if !defined(__SUNPRO_CC) || __SUNPRO_CC > 0x420
ATTRIBUTE_EXPORT void * FLATTEN operator new (size_t sz, const std::nothrow_t&) throw() {
  return xxmalloc(sz);
}

ATTRIBUTE_EXPORT void * FLATTEN operator new[] (size_t size)
#if defined(_GLIBCXX_THROW)
  _GLIBCXX_THROW (std::bad_alloc)
#endif
{
  void * ptr = xxmalloc(size);
  if (ptr) {
    return ptr;
  }
  throw std::bad_alloc();
}

ATTRIBUTE_EXPORT void * FLATTEN operator new[] (size_t sz, const std::nothrow_t&)
  throw()
 {
  return xxmalloc(sz);
}

ATTRIBUTE_EXPORT void FLATTEN operator delete[] (void * ptr)
#if defined(_GLIBCXX_USE_NOEXCEPT)
  _GLIBCXX_USE_NOEXCEPT
#else
#if defined(__GNUC__)
  // clang + libcxx on linux
  _NOEXCEPT
#endif
#endif
{
  CUSTOM_FREE (ptr);
}

#if defined(__cpp_sized_deallocation) && __cpp_sized_deallocation >= 201309

ATTRIBUTE_EXPORT void FLATTEN operator delete(void * ptr, size_t)
#if !defined(linux_)
  throw ()
#endif
{
  CUSTOM_FREE (ptr);
}

ATTRIBUTE_EXPORT void FLATTEN operator delete[](void * ptr, size_t)
#if defined(__GNUC__)
  _GLIBCXX_USE_NOEXCEPT
#endif
{
  CUSTOM_FREE (ptr);
}
#endif

// --- Aligned new/delete (C++17) ---
#if defined(__cpp_aligned_new) && __cpp_aligned_new >= 201606
ATTRIBUTE_EXPORT void* FLATTEN operator new(std::size_t n, std::align_val_t al) {
  void* p = CUSTOM_MEMALIGN(n, al); 
  if (!p) throw std::bad_alloc();
  return p;
}
ATTRIBUTE_EXPORT void* FLATTEN operator new[](std::size_t n, std::align_val_t al) {
  void* p = CUSTOM_MEMALIGN(n, al); 
  if (!p) throw std::bad_alloc();
  return p;
}
ATTRIBUTE_EXPORT void FLATTEN operator delete(void* p, std::align_val_t) noexcept { CUSTOM_FREE(p); }
ATTRIBUTE_EXPORT void FLATTEN operator delete[](void* p, std::align_val_t) noexcept { CUSTOM_FREE(p); }

// nothrow aligned new/delete
ATTRIBUTE_EXPORT void* FLATTEN operator new(std::size_t n, std::align_val_t al, const std::nothrow_t&) noexcept {
  try { return ::operator new(n, al); } catch (...) { return nullptr; }
}
ATTRIBUTE_EXPORT void* FLATTEN operator new[](std::size_t n, std::align_val_t al, const std::nothrow_t&) noexcept {
  try { return ::operator new[](n, al); } catch (...) { return nullptr; }
}
ATTRIBUTE_EXPORT void FLATTEN operator delete(void* p, std::align_val_t al, const std::nothrow_t&) noexcept {
  CUSTOM_FREE(p);
}
ATTRIBUTE_EXPORT void FLATTEN operator delete[](void* p, std::align_val_t al, const std::nothrow_t&) noexcept {
  CUSTOM_FREE(p);
}

// sized aligned delete (if both features present)
#if defined(__cpp_sized_deallocation) && __cpp_sized_deallocation >= 201309
ATTRIBUTE_EXPORT void FLATTEN operator delete(void* p, std::size_t, std::align_val_t) noexcept { CUSTOM_FREE(p); }
ATTRIBUTE_EXPORT void FLATTEN operator delete[](void* p, std::size_t, std::align_val_t) noexcept { CUSTOM_FREE(p); }
#endif
#endif


#endif
#endif

/***** replacement functions for GNU libc extensions to malloc *****/

extern "C" void * MYCDECL CUSTOM_VALLOC (size_t sz)
{
  return CUSTOM_MEMALIGN (HL::CPUInfo::PageSize, sz); // Default page size on most architectures.
}


extern "C" void * MYCDECL CUSTOM_PVALLOC (size_t sz)
{
  size_t pgsize_minus1 = HL::CPUInfo::PageSize - 1;
  // Rounds up to the next pagesize and then calls valloc. Hoard
  // doesn't support aligned memory requests.
  return CUSTOM_VALLOC ((sz + pgsize_minus1) & ~pgsize_minus1);
}

// The wacky recalloc function, for Windows.
extern "C" void * MYCDECL CUSTOM_RECALLOC (void * p, size_t num, size_t sz)
{
  auto n = num * sz;
  void * ptr = CUSTOM_REALLOC (p, n);
  if (ptr) {
    // Clear out the memory.
    memset (ptr, 0, n);
  }
  return ptr;
}

#if defined(_WIN32)

/////// Other replacement functions that call malloc.

// from http://msdn2.microsoft.com/en-us/library/6ewkz86d(VS.80).aspx
// fgetc, _fgetchar, fgets, fprintf, fputc, _fputchar, fputs, fread, fscanf, fseek, fsetpos
// _fullpath, fwrite, getc, getchar, _getcwd, _getdcwd, gets, _getw, _popen, printf, putc
// putchar, _putenv, puts, _putw, scanf, _searchenv, setvbuf, _strdup, system, _tempnam,
// ungetc, vfprintf, vprintf


char * CUSTOM_GETENV(const char * str) {
  char buf[32767];
  int len = GetEnvironmentVariable (str, buf, 32767);
  if (len == 0) {
    return nullptr;
  } else {
    char * str = new char[len + 1];
    strncpy (str, buf, len + 1);
    return str;
  }
}

int CUSTOM_PUTENV(char * str) {
  char * eqpos = strchr (str, '=');
  if (eqpos) {
    char first[32767], second[32767];
    int namelen = (size_t) eqpos - (size_t) str;
    strncpy (first, str, namelen);
    first[namelen] = '\0';
    int valuelen = strlen (eqpos + 1);
    strncpy (second, eqpos + 1, valuelen);
    second[valuelen] = '\0';
    char buf[255];
    sprintf (buf, "setting %s to %s\n", first, second);
    printf (buf);
    SetEnvironmentVariable (first, second);
    return 0;
  }
  return -1;
}

#endif

#if defined(__GNUC__) && defined(__GLIBC__) && !defined(__FreeBSD__) && !defined(__NetBSD__)
extern "C" void* __libc_malloc(size_t n)        __attribute__((visibility("default")));
extern "C" void  __libc_free(void* p)           __attribute__((visibility("default")));
extern "C" void* __libc_calloc(size_t a,size_t b) __attribute__((visibility("default")));
extern "C" void* __libc_realloc(void* p,size_t n) __attribute__((visibility("default")));
extern "C" void* __libc_memalign(size_t m,size_t n) __attribute__((visibility("default")));

extern "C" void* __libc_malloc(size_t n){ return CUSTOM_MALLOC(n); }
extern "C" void  __libc_free(void* p){     CUSTOM_FREE(p); }
extern "C" void* __libc_calloc(size_t a,size_t b){ return CUSTOM_CALLOC(a,b); }
extern "C" void* __libc_realloc(void* p,size_t n){ return CUSTOM_REALLOC(p,n); }
extern "C" void* __libc_memalign(size_t m,size_t n){ return CUSTOM_MEMALIGN(m,n); }
#endif

