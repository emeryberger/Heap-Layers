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
#include <sys/cdefs.h>

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

#define WEAK(x) __attribute__ ((weak, alias(#x)))
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

#define WEAK_REDEF1(type,fname,arg1) ATTRIBUTE_EXPORT type fname(arg1) __THROW WEAK(custom##fname)
#define WEAK_REDEF2(type,fname,arg1,arg2) ATTRIBUTE_EXPORT type fname(arg1,arg2) __THROW WEAK(custom##fname)
#define WEAK_REDEF2_NOTHROW(type,fname,arg1,arg2) ATTRIBUTE_EXPORT type fname(arg1,arg2) WEAK(custom##fname)
#define WEAK_REDEF3(type,fname,arg1,arg2,arg3) ATTRIBUTE_EXPORT type fname(arg1,arg2,arg3) __THROW WEAK(custom##fname)

extern "C" {
  WEAK_REDEF1(void *, malloc, size_t);
  WEAK_REDEF1(void, free, void *);
  WEAK_REDEF1(void, cfree, void *);
  WEAK_REDEF2(void *, calloc, size_t, size_t);
  WEAK_REDEF2(void *, realloc, void *, size_t);
  WEAK_REDEF3(void *, reallocarray, void *, size_t, size_t);
  WEAK_REDEF2(void *, memalign, size_t, size_t);
  WEAK_REDEF3(int, posix_memalign, void **, size_t, size_t);
#ifdef __USE_XOPEN2K // a work-around for an exception anomaly
  //  WEAK_REDEF2_NOTHROW(void *, aligned_alloc, size_t, size_t);
#else
  WEAK_REDEF2(void *, aligned_alloc, size_t, size_t);
#endif
  WEAK_REDEF1(size_t, malloc_usable_size, void *);
}

#include "wrapper.cpp"
#include "gnuwrapper-hooks.cpp"
