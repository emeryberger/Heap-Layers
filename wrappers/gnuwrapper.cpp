// -*- C++ -*-

/**
 * @file   gnuwrapper.cpp
 * @brief  Replaces malloc family on GNU/Linux with custom versions.
 * @author Emery Berger <http://www.cs.umass.edu/~emery>
 * @note   Copyright (C) 2010 by Emery Berger, University of Massachusetts Amherst.
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
  - xxmalloc_usable_size
  - xxmalloc_lock
  - xxmalloc_unlock

  See the extern "C" block below for function prototypes and more
  details. YOU SHOULD NOT NEED TO MODIFY ANY OF THE CODE HERE TO
  SUPPORT ANY ALLOCATOR.


  LIMITATIONS:

  - This wrapper assumes that the underlying allocator will do "the
    right thing" when xxfree() is called with a pointer internal to an
    allocated object. Header-based allocators, for example, need not
    apply.

*/


#define WEAK(x) __attribute__ ((weak, alias(#x)))
#ifndef __THROW
#define __THROW
#endif

#define CUSTOM_PREFIX(x) hoard_##x

#define WEAK_REDEF1(type,fname,arg1) type fname(arg1) __THROW WEAK(hoard_##fname)
#define WEAK_REDEF2(type,fname,arg1,arg2) type fname(arg1,arg2) __THROW WEAK(hoard_##fname)
#define WEAK_REDEF3(type,fname,arg1,arg2,arg3) type fname(arg1,arg2,arg3) __THROW WEAK(hoard_##fname)

extern "C" {
  WEAK_REDEF1(void *, malloc, size_t);
  WEAK_REDEF1(void, free, void *);
  WEAK_REDEF1(void, cfree, void *);
  WEAK_REDEF2(void *, calloc, size_t, size_t);
  WEAK_REDEF2(void *, realloc, void *, size_t);
  WEAK_REDEF2(void *, memalign, size_t, size_t);
  WEAK_REDEF3(int, posix_memalign, void **, size_t, size_t);
  WEAK_REDEF2(void *, aligned_alloc, size_t, size_t);
  WEAK_REDEF1(size_t, malloc_usable_size, void *);
}

#if 0
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
#endif

#include "wrapper.cpp"
