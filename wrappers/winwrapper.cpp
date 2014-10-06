/* -*- C++ -*- */

/*
 * @file winwrapper.cpp
 * @brief  Replaces malloc family on Windows with custom versions.
 * @author Emery Berger <http://www.cs.umass.edu/~emery>
 * @note   Copyright (C) 2011-2014 by Emery Berger, University of Massachusetts Amherst.
 */

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

  - This wrapper also assumes that there is some way to lock all the
    heaps used by a given allocator; however, such support is only
    required by programs that also call fork(). In case your program
    does not, the lock and unlock calls given below can be no-ops.

*/

#include "x86jump.h"

extern "C" {

  void * xxmalloc (size_t);
  void   xxfree (void *);

  // Takes a pointer and returns how much space it holds.
  size_t xxmalloc_usable_size (void *);

  // Locks the heap(s), used prior to any invocation of fork().
  void xxmalloc_lock (void);

  // Unlocks the heap(s), after fork().
  void xxmalloc_unlock (void);

}


#ifdef _DEBUG
#error "This library must be compiled in release mode."
#endif	

#include <windows.h>

#define WIN32_LEAN_AND_MEAN

#if (_WIN32_WINNT < 0x0500)
#define _WIN32_WINNT 0x0500
#endif

#pragma inline_depth(255)

#pragma warning(disable: 4273)
#pragma warning(disable: 4098)  // Library conflict.
#pragma warning(disable: 4355)  // 'this' used in base member initializer list.
#pragma warning(disable: 4074)	// initializers put in compiler reserved area.


#define WINWRAPPER_PREFIX(x) winwrapper_##x

typedef struct
{
  const char *import;		// import name of patch routine
  FARPROC replacement;		// pointer to replacement function
  FARPROC original;		// pointer to original function
  unsigned char codebytes[sizeof(X86Jump)];	// original code storage
} PATCH;

static bool PatchMeIn();

#include <stdio.h>

extern "C" void InitializeWinWrapper() {
  PatchMeIn();
}

extern "C" void FinalizeWinWrapper() {
  HeapAlloc (GetProcessHeap(), 0, 1);
}


extern "C" {

  __declspec(dllexport) int ReferenceWinWrapperStub;

  void * WINWRAPPER_PREFIX(expand) (void * ptr) {
    return NULL;
  }

  static void * WINWRAPPER_PREFIX(realloc) (void * ptr, size_t sz) 
  {
    // NULL ptr = malloc.
    if (ptr == NULL) {
      return xxmalloc(sz);
    }

    // 0 size = free. We return a small object.  This behavior is
    // apparently required under Mac OS X and optional under POSIX.
    if (sz == 0) {
      xxfree (ptr);
      return xxmalloc(1);
    }

    size_t originalSize = xxmalloc_usable_size (ptr);
    size_t minSize = (originalSize < sz) ? originalSize : sz;

    // Don't change size if the object is shrinking by less than half.
    if ((originalSize / 2 < sz) && (sz <= originalSize)) {
      // Do nothing.
      return ptr;
    }

    void * buf = xxmalloc (sz);

    if (buf != NULL) {
      // Successful malloc.
      // Copy the contents of the original object
      // up to the size of the new block.
      memcpy (buf, ptr, minSize);
      xxfree (ptr);
    }

    // Return a pointer to the new one.
    return buf;
  }

  static void * WINWRAPPER_PREFIX(recalloc)(void * memblock, size_t num, size_t size) {
    const auto requestedSize = num * size;
    void * ptr = WINWRAPPER_PREFIX(realloc)(memblock, requestedSize);
    if (ptr != nullptr) {
      const auto actualSize = xxmalloc_usable_size(ptr);
      if (actualSize > requestedSize) {
	// Clear out any memory after the end of the requested chunk.
	memset (static_cast<char *>(ptr) + requestedSize, 0, actualSize - requestedSize);
      }
    }
    return ptr;
  }

  static void * WINWRAPPER_PREFIX(calloc)(size_t num, size_t size) {
    void * ptr = xxmalloc (num * size);
    if (ptr) {
      memset (ptr, 0, xxmalloc_usable_size(ptr));
    }
    return ptr;
  }

  // WINWRAPPER_PREFIX(putenv)
  // WINWRAPPER_PREFIX(getenv)

  char * WINWRAPPER_PREFIX(strdup) (const char * s)
  {
    char * newString = NULL;
    if (s != NULL) {
      int len = strlen(s) + 1;
      if ((newString = (char *) xxmalloc(len))) {
	memcpy (newString, s, len);
      }
    }
    return newString;
  }

}


/* ------------------------------------------------------------------------ */

static PATCH rls_patches[] = 
  {
    {"strdup",		(FARPROC) WINWRAPPER_PREFIX(strdup),   0},
    {"_expand",		(FARPROC) WINWRAPPER_PREFIX(expand),   0},

    // operator new, new[], delete, delete[].
    
#ifdef _WIN64
    
    {"??2@YAPEAX_K@Z",  (FARPROC) xxmalloc,    0},
    {"??_U@YAPEAX_K@Z", (FARPROC) xxmalloc,    0},
    {"??3@YAXPEAX@Z",   (FARPROC) xxfree,      0},
    {"??_V@YAXPEAX@Z",  (FARPROC) xxfree,      0},

#else

    {"??2@YAPAXI@Z",    (FARPROC) xxmalloc,    0},
    {"??_U@YAPAXI@Z",   (FARPROC) xxmalloc,    0},
    {"??3@YAXPAX@Z",    (FARPROC) xxfree,      0},
    {"??_V@YAXPAX@Z",   (FARPROC) xxfree,      0},

#endif

    // the nothrow variants new, new[].

    {"??2@YAPAXIABUnothrow_t@std@@@Z",  (FARPROC) xxmalloc, 0},
    {"??_U@YAPAXIABUnothrow_t@std@@@Z", (FARPROC) xxmalloc, 0},
    
    {"_msize",	(FARPROC) xxmalloc_usable_size,    	0},
    {"calloc",	(FARPROC) WINWRAPPER_PREFIX(calloc),	0},
    {"_calloc_base",(FARPROC) WINWRAPPER_PREFIX(calloc),0},
    {"_calloc_crt",(FARPROC) WINWRAPPER_PREFIX(calloc),	0},
    {"malloc",	(FARPROC) xxmalloc,			0},
    {"_malloc_base",(FARPROC) xxmalloc,			0},
    {"_malloc_crt",(FARPROC) xxmalloc,			0},
    {"realloc",	(FARPROC) WINWRAPPER_PREFIX(realloc),	0},
    {"_realloc_base",(FARPROC) WINWRAPPER_PREFIX(realloc),0},
    {"_realloc_crt",(FARPROC) WINWRAPPER_PREFIX(realloc),0},
    {"free",	(FARPROC) xxfree,                  	0},
    {"_free_base",(FARPROC) xxfree,                  	0},
    {"_free_crt",(FARPROC) xxfree,                  	0},
    {"_recalloc", (FARPROC) WINWRAPPER_PREFIX(recalloc),0},
    {"_recalloc_base", (FARPROC) WINWRAPPER_PREFIX(recalloc),0}
    {"_recalloc_crt", (FARPROC) WINWRAPPER_PREFIX(recalloc),0}
  };


static void PatchIt (PATCH *patch)
{
  // Change rights on CRT Library module to execute/read/write.

  MEMORY_BASIC_INFORMATION mbi_thunk;
  VirtualQuery((void*)patch->original, &mbi_thunk, 
	       sizeof(MEMORY_BASIC_INFORMATION));
  VirtualProtect(mbi_thunk.BaseAddress, mbi_thunk.RegionSize, 
		 PAGE_EXECUTE_READWRITE, &mbi_thunk.Protect);

  // Patch CRT library original routine:
  // 	save original code bytes for exit restoration
  //		write jmp <patch_routine> (at least 5 bytes long) to original.

  memcpy (patch->codebytes, patch->original, sizeof(X86Jump));
  unsigned char *patchloc = (unsigned char*)patch->original;
  new (patchloc) X86Jump (patch->replacement);
	
  // Reset CRT library code to original page protection.

  VirtualProtect(mbi_thunk.BaseAddress, mbi_thunk.RegionSize, 
		 mbi_thunk.Protect, &mbi_thunk.Protect);
}


static bool PatchMeIn (void)
{
  bool patchedIn = false;

  // Library names. We check all of these at runtime and link ourselves in.
  static const char * RlsCRTLibraryName[] = {"MSVCR71.DLL", "MSVCR80.DLL", "MSVCR90.DLL", "MSVCR100.DLL", "MSVCP100.DLL", "MSVCR110.DLL", "MSVCP110.DLL", "MSVCR120.DLL", "MSVCP120.DLL", "MSVCR140.DLL", "MSVCP140.DLL", "APPCRT140.DLL", "MSVCRT.DLL" };
  
  static const int RlsCRTLibraryNameLength = sizeof(RlsCRTLibraryName) / sizeof(const char *);

  // Acquire the module handles for the CRT libraries.
  for (int i = 0; i < RlsCRTLibraryNameLength; i++) {

    HMODULE RlsCRTLibrary = GetModuleHandleA(RlsCRTLibraryName[i]);

    HMODULE DefCRTLibrary = 
      RlsCRTLibrary;

    // Patch all relevant release CRT Library entry points.
    if (RlsCRTLibrary) {
      for (int j = 0; j < sizeof(rls_patches) / sizeof(*rls_patches); j++) {
	if (rls_patches[j].original = GetProcAddress(RlsCRTLibrary, rls_patches[j].import)) {
	  PatchIt(&rls_patches[j]);
 	  patchedIn = true;
	}
      }
    }
  }
  return patchedIn;
}

