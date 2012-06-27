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

/*
  To use this library,
  you only need to define the following allocation functions:
  
  - xxmalloc
  - xxfree
  - xxmalloc_usable_size
  
  See the extern "C" block below for function prototypes and more
  details. YOU SHOULD NOT NEED TO MODIFY ANY OF THE CODE HERE TO
  SUPPORT ANY ALLOCATOR.


  LIMITATIONS:

  - This wrapper assumes that the underlying allocator will do "the
    right thing" when xxfree() is called with a pointer internal to an
    allocated object. Header-based allocators, for example, need not
    apply.

*/

static bool initialized = false;

extern "C" {

  void * xxmalloc (size_t);
  void   xxfree (void *);

  // Takes a pointer and returns how much space it holds.
  size_t xxmalloc_usable_size (void *);


  static void my_init_hook (void);

  // New hooks for allocation functions.
  static void * my_malloc_hook (size_t, const void *);
  static void   my_free_hook (void *, const void *);
  static void * my_realloc_hook (void *, size_t, const void *);
  static void * my_memalign_hook (size_t, size_t, const void *);

  // Store the old hooks just in case.
  static void * (*old_malloc_hook) (size_t, const void *);
  static void   (*old_free_hook) (void *, const void *);
  static void * (*old_realloc_hook)(void *ptr, size_t size, const void *caller);
  static void * (*old_memalign_hook)(size_t alignment, size_t size, const void *caller);

// From GNU libc 2.14 this macro is defined, to declare
// hook variables as volatile. Define it as empty for
// older glibc versions
#ifndef __MALLOC_HOOK_VOLATILE
 #define __MALLOC_HOOK_VOLATILE
#endif

  void (*__MALLOC_HOOK_VOLATILE __malloc_initialize_hook) (void) = my_init_hook;

  static void my_init_hook (void) {
    // Store the old hooks.
    old_malloc_hook = __malloc_hook;
    old_free_hook = __free_hook;
    old_realloc_hook = __realloc_hook;
    old_memalign_hook = __memalign_hook;

    // Point the hooks to the replacement functions.
    __malloc_hook = my_malloc_hook;
    __free_hook = my_free_hook;
    __realloc_hook = my_realloc_hook;
    __memalign_hook = my_memalign_hook;

    initialized = true;
  }

  static void * my_malloc_hook (size_t size, const void *) {
    return xxmalloc(size);
  }

  static void my_free_hook (void * ptr, const void *) {
    xxfree(ptr);
  }

  static void * my_realloc_hook (void * ptr, size_t sz, const void *) {
    // NULL ptr = malloc.
    if (ptr == NULL) {
      return xxmalloc(sz);
    }

    if (sz == 0) {
      xxfree (ptr);
#if defined(__APPLE__)
      // 0 size = free. We return a small object.  This behavior is
      // apparently required under Mac OS X and optional under POSIX.
      return xxmalloc(1);
#else
      // For POSIX, don't return anything.
      return NULL;
#endif
    }

    size_t objSize = xxmalloc_usable_size(ptr);

#if 0
    // Custom logic here to ensure we only do a logarithmic number of
    // reallocations (with a constant space overhead).

    // Don't change size if the object is shrinking by less than half.
    if ((objSize / 2 < sz) && (sz <= objSize)) {
      // Do nothing.
      return ptr;
    }
    // If the object is growing by less than 2X, double it.
    if ((objSize < sz) && (sz < objSize * 2)) {
      sz = objSize * 2;
    }
#endif

    void * buf = xxmalloc(sz);

    if (buf != NULL) {
      // Successful malloc.
      // Copy the contents of the original object
      // up to the size of the new block.
      size_t minSize = (objSize < sz) ? objSize : sz;
      memcpy (buf, ptr, minSize);
      xxfree (ptr);
    }

    // Return a pointer to the new one.
    return buf;
  }

  static void * my_memalign_hook (size_t size, size_t alignment, const void *) {
    // Check for non power-of-two alignment, or mistake in size.
    if ((alignment == 0) ||
	(alignment & (alignment - 1)))
      {
	return NULL;
      }

    // Try to just allocate an object of the requested size.
    // If it happens to be aligned properly, just return it.
    void * ptr = xxmalloc (size);
    if (((size_t) ptr & (alignment - 1)) == (size_t) ptr) {
      // It is already aligned just fine; return it.
      return ptr;
    }

    // It was not aligned as requested: free the object.
    xxfree (ptr);

    // Now get a big chunk of memory and align the object within it.
    // NOTE: this REQUIRES that the underlying allocator be able
    // to free the aligned object, or ignore the free request.
    void * buf = xxmalloc (2 * alignment + size);
    void * alignedPtr = (void *) (((size_t) buf + alignment - 1) & ~(alignment - 1));

    return alignedPtr;
  }


  // This is here because, for some reason, the GNU hooks don't
  // necessarily replace all memory operations as they should.

  int posix_memalign (void **memptr, size_t alignment, size_t size)
  {
    if (!initialized) {
      my_init_hook();
    }
    // Check for non power-of-two alignment.
    if ((alignment == 0) ||
	(alignment & (alignment - 1)))
      {
	return EINVAL;
      }
    void * ptr = my_memalign_hook (size, alignment, NULL);
    if (!ptr) {
      return ENOMEM;
    } else {
      *memptr = ptr;
      return 0;
    }
  }

  size_t malloc_usable_size (void * ptr) {
    return xxmalloc_usable_size (ptr);
  }

}

