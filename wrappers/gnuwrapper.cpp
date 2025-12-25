// -*- C++ -*-

/**
 * @file   gnuwrapper.cpp
 * @brief  Replaces malloc family on GNU/Linux with custom versions.
 * @author Emery Berger <http://www.emeryberger.com>
 * @note   Copyright (C) 2010-2020 by Emery Berger.

  Heap Layers is distributed under the terms of the Apache 2.0 license.

  You may obtain a copy of the License at
  http://www.apache.org/licenses/LICENSE-2.0

 */


#ifndef __GNUC__
#error "This file requires the GNU compiler."
#endif

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <new>
#include <pthread.h>

#include "heaplayers.h"

/*
  To use this library,
  you only need to define the following allocation functions:
  
  - xxmalloc
  - xxfree
  - xxmemalign
  - xxmalloc_usable_size
  - xxmalloc_lock
  - xxmalloc_unlock

  See the extern "C" block below for function prototypes and more
  details. YOU SHOULD NOT NEED TO MODIFY ANY OF THE CODE HERE TO
  SUPPORT ANY ALLOCATOR.

*/

#ifndef __THROW
#define __THROW
#endif

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmacro-redefined"
#endif

#define CUSTOM_PREFIX(x) custom##x

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

#define ATTRIBUTE_EXPORT __attribute__((visibility("default")))
#define STRONG_ALIAS(target) __attribute__((alias(#target), visibility("default")))


#if 0 // def __GLIBC__
// Export the public names with glibc's default version node.
__asm__(".symver custommalloc,        malloc@@GLIBC_2.2.5");
__asm__(".symver customfree,          free@@GLIBC_2.2.5");
__asm__(".symver customcalloc,        calloc@@GLIBC_2.2.5");
__asm__(".symver customrealloc,       realloc@@GLIBC_2.2.5");
__asm__(".symver customposix_memalign,posix_memalign@@GLIBC_2.2.5");
__asm__(".symver custommemalign,      memalign@@GLIBC_2.2.5");
__asm__(".symver customaligned_alloc, aligned_alloc@@GLIBC_2.2.5");
__asm__(".symver customstrdup,        strdup@@GLIBC_2.2.5");

#else

#define STRONG_REDEF1(type,fname,arg1) ATTRIBUTE_EXPORT type fname(arg1) __THROW STRONG_ALIAS(custom##fname)
#define STRONG_REDEF2(type,fname,arg1,arg2) ATTRIBUTE_EXPORT type fname(arg1,arg2) __THROW STRONG_ALIAS(custom##fname)
#define STRONG_REDEF3(type,fname,arg1,arg2,arg3) ATTRIBUTE_EXPORT type fname(arg1,arg2,arg3) __THROW STRONG_ALIAS(custom##fname)

extern "C" {
  STRONG_REDEF1(void *, malloc, size_t);
  STRONG_REDEF1(void, free, void *);
  STRONG_REDEF1(void, cfree, void *);
  STRONG_REDEF2(void *, calloc, size_t, size_t);
  STRONG_REDEF2(void *, realloc, void *, size_t);
  STRONG_REDEF3(void *, reallocarray, void *, size_t, size_t);
  STRONG_REDEF2(void *, memalign, size_t, size_t);
  STRONG_REDEF3(int, posix_memalign, void **, size_t, size_t);
  STRONG_REDEF2(void *, aligned_alloc, size_t, size_t);
  STRONG_REDEF1(size_t, malloc_usable_size, void *);
  STRONG_REDEF1(char *, strdup, const char *);
  STRONG_REDEF2(char *, strndup, const char *, size_t);
  STRONG_REDEF1(void *, valloc, size_t);
  STRONG_REDEF1(void *, pvalloc, size_t);
}

#endif

#include "wrapper.cpp"
