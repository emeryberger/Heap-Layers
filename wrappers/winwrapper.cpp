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

#include <windows.h>
#include <errno.h>
#include <psapi.h>
#include <tchar.h>

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
#include <stdio.h>

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

class Patch {
public:
  const char *import;		// import name of patch routine
  FARPROC replacement;		// pointer to replacement function
  FARPROC original;		// pointer to original function
  bool patched;                 // did we actually execute this patch?
  unsigned char codebytes[sizeof(X86Jump)];	// original code storage
};

static bool PatchMe();
static void UnpatchMe();
extern "C" void executeRegisteredFunctions();

extern "C" void InitializeWinWrapper() {
  // Allocate (and leak) something from the old Windows heap.
  HeapAlloc (GetProcessHeap(), 0, 1);
  PatchMe();
  // Allocations after this point are from the replaced heap.
}

extern "C" void FinalizeWinWrapper() {
  // For now, we don't execute the registered functions.
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

    auto originalSize = xxmalloc_usable_size (ptr);
    auto minSize = (originalSize < sz) ? originalSize : sz;

    // Don't change size if the object is shrinking by less than half.
    if ((originalSize / 2 < sz) && (sz <= originalSize)) {
      // Do nothing.
      return ptr;
    }

    auto * buf = xxmalloc (sz);

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
    auto * ptr = WINWRAPPER_PREFIX(realloc)(memblock, requestedSize);
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
    auto * ptr = xxmalloc (num * size);
    if (ptr) {
      memset (ptr, 0, xxmalloc_usable_size(ptr));
    }
    return ptr;
  }

  char * WINWRAPPER_PREFIX(strdup) (const char * s)
  {
    char * newString = NULL;
    if (s != NULL) {
      auto len = strlen(s) + 1;
      if ((newString = (char *) xxmalloc(len))) {
	memcpy (newString, s, len);
      }
    }
    return newString;
  }

  const int MAX_EXIT_FUNCTIONS = 256;
  _onexit_t exitFunctions[MAX_EXIT_FUNCTIONS];
  static int exitFunctionsRegistered = 0;

  _onexit_t bogus_onexit(_onexit_t fn) {
    if (exitFunctionsRegistered == MAX_EXIT_FUNCTIONS) {
      return NULL;
    } else {
      exitFunctions[exitFunctionsRegistered] = fn;
      exitFunctionsRegistered++;
      return fn;
    }
  }

  int bogus_atexit(void (*fn)(void)) {
    if (bogus_onexit((_onexit_t) fn) == NULL) {
      return ENOMEM;
    } else {
      return 0;
    }
  }

  void executeRegisteredFunctions() {
    for (int i = exitFunctionsRegistered; i >= 0; i--) {
      (*exitFunctions[i])();
    }
  }

  void * WINWRAPPER_PREFIX(_calloc_dbg)(size_t num,
					size_t size,
					int blockType,
					const char *filename,
					int linenumber) 
  {
    return WINWRAPPER_PREFIX(calloc)(num, size);
  }

  void * WINWRAPPER_PREFIX(_malloc_dbg)(size_t size,
					int blockType,
					const char *filename,
					int linenumber)
  {
    return xxmalloc(size);
  }

  void * WINWRAPPER_PREFIX(_realloc_dbg)(void *userData,
					 size_t newSize,
					 int blockType,
					 const char *filename,
					 int linenumber)
  {
    return WINWRAPPER_PREFIX(realloc)(userData, newSize);
  }

  void WINWRAPPER_PREFIX(_free_dbg)(void *userData,
				    int blockType)
  {
    xxfree(userData);
  }

  LPVOID WINWRAPPER_PREFIX(HeapAlloc)(HANDLE hHeap,
				      DWORD dwFlags,
				      SIZE_T dwBytes)
  {
    if (hHeap == NULL) {
      return NULL;
    }
    if (dwFlags & HEAP_ZERO_MEMORY) {
      return WINWRAPPER_PREFIX(calloc)(1, dwBytes);
    } else {
      return xxmalloc(dwBytes);
    }
  }

  LPVOID WINAPI WINWRAPPER_PREFIX(HeapReAlloc)(HANDLE hHeap,
					       DWORD  dwFlags,
					       LPVOID lpMem,
					       SIZE_T dwBytes)
  {
    // TO DO: implement HEAP_REALLOC_IN_PLACE_ONLY and HEAP_ZERO_MEMORY.
    return WINWRAPPER_PREFIX(realloc)(lpMem, dwBytes);
  }
  
  BOOL WINWRAPPER_PREFIX(HeapFree)(HANDLE hHeap,
				   DWORD dwFlags,
				   LPVOID lpMem)
  {
    xxfree(lpMem);
    return true;
  }

  BOOL WINAPI WINWRAPPER_PREFIX(HeapValidate)(HANDLE  hHeap,
					      DWORD   dwFlags,
					      LPCVOID lpMem) 
  {
    // A stub that says the heap is fine.
    return TRUE;
  }

  SIZE_T WINAPI WINWRAPPER_PREFIX(HeapSize)(HANDLE  hHeap,
					    DWORD   dwFlags,
					    LPCVOID lpMem)
  {
    return xxmalloc_usable_size((void *) lpMem);
  }

  BOOL WINAPI WINWRAPPER_PREFIX(HeapWalk)(HANDLE hHeap,
					  LPPROCESS_HEAP_ENTRY lpEntry)
  {
    // A stub that prevents actually walking the heap.
    return FALSE;
  }

  HANDLE WINAPI WINWRAPPER_PREFIX(HeapCreate)(DWORD  flOptions,
					      SIZE_T dwInitialSize,
					      SIZE_T dwMaximumSize)
  {
    // Ignore all options and just return a bogus (non-null) handle.
    return (HANDLE) 0x1;
  }

  BOOL WINAPI WINWRAPPER_PREFIX(HeapDestroy)(HANDLE hHeap) 
  {
    // For now, do nothing and claim we destroyed the heap.
    // NOTE: this potentially leaks memory if the user is creating
    // and destroying heaps rather than explicitly freeing objects.
    return TRUE;
  }

}


/* ------------------------------------------------------------------------ */

static Patch rls_patches[] = 
  {
    // operator new, new[], delete, delete[].
    
    //#ifdef _WIN64
    
    {"??2@YAPEAX_K@Z",  (FARPROC) xxmalloc,    false, 0},
    {"??_U@YAPEAX_K@Z", (FARPROC) xxmalloc,    false, 0},
    {"??3@YAXPEAX@Z",   (FARPROC) xxfree,      false, 0},
    {"??_V@YAXPEAX@Z",  (FARPROC) xxfree,      false, 0},

    //#else

    {"??2@YAPAXI@Z",    (FARPROC) xxmalloc,    false, 0},
    {"??_U@YAPAXI@Z",   (FARPROC) xxmalloc,    false, 0},
    {"??3@YAXPAX@Z",    (FARPROC) xxfree,      false, 0},
    {"??_V@YAXPAX@Z",   (FARPROC) xxfree,      false, 0},

    //#endif

    // Debug versions.
    {"_calloc_dbg",(FARPROC) WINWRAPPER_PREFIX(_calloc_dbg),	false, 0},
    {"_malloc_dbg",(FARPROC) WINWRAPPER_PREFIX(_malloc_dbg),	false, 0},
    {"_realloc_dbg",(FARPROC) WINWRAPPER_PREFIX(_realloc_dbg),  false, 0},
    {"_free_dbg",(FARPROC) WINWRAPPER_PREFIX(_free_dbg),      	false, 0},

    // the nothrow variants new, new[], delete, delete[]

    {"??2@YAPAXIABUnothrow_t@std@@@Z",  (FARPROC) xxmalloc, false, 0},
    {"??_U@YAPAXIABUnothrow_t@std@@@Z", (FARPROC) xxmalloc, false, 0},
    {"??3@YAXPAXABUnothrow_t@std@@@Z",  (FARPROC) xxfree, false, 0},
    {"??_V@YAXPAXABUnothrow_t@std@@@Z", (FARPROC) xxfree, false, 0},
  
    {"_msize",	(FARPROC) xxmalloc_usable_size,    	false, 0},
    {"calloc",	(FARPROC) WINWRAPPER_PREFIX(calloc),	false, 0},
    {"_calloc_base",(FARPROC) WINWRAPPER_PREFIX(calloc),false, 0},
    {"_calloc_crt",(FARPROC) WINWRAPPER_PREFIX(calloc),	false, 0},
    {"malloc",	(FARPROC) xxmalloc,			false, 0},
    {"_malloc_base",(FARPROC) xxmalloc,			false, 0},
    {"_malloc_crt",(FARPROC) xxmalloc,			false, 0},
    {"realloc",	(FARPROC) WINWRAPPER_PREFIX(realloc),	false, 0},
    {"_realloc_base",(FARPROC) WINWRAPPER_PREFIX(realloc),false, 0},
    {"_realloc_crt",(FARPROC) WINWRAPPER_PREFIX(realloc),false, 0},
    {"free",	(FARPROC) xxfree,                  	false, 0},
    {"_free_base",(FARPROC) xxfree,                  	false, 0},
    {"_free_crt",(FARPROC) xxfree,                  	false, 0},
    {"_recalloc", (FARPROC) WINWRAPPER_PREFIX(recalloc),false, 0},
    {"_recalloc_base", (FARPROC) WINWRAPPER_PREFIX(recalloc),false, 0},
    {"_recalloc_crt", (FARPROC) WINWRAPPER_PREFIX(recalloc),false, 0},

    {"_onexit", (FARPROC) bogus_onexit, false, 0},
    {"atexit", (FARPROC) bogus_atexit, false, 0},

    {"_expand",		(FARPROC) WINWRAPPER_PREFIX(expand),   false, 0},
    {"strdup",		(FARPROC) WINWRAPPER_PREFIX(strdup),   false, 0},

    {"HeapAlloc",	(FARPROC) WINWRAPPER_PREFIX(HeapAlloc),false, 0},
    {"HeapFree",	(FARPROC) WINWRAPPER_PREFIX(HeapFree),false, 0},
    {"HeapReAlloc",	(FARPROC) WINWRAPPER_PREFIX(HeapReAlloc),false, 0},
    {"HeapValidate",	(FARPROC) WINWRAPPER_PREFIX(HeapValidate),false, 0},
    {"HeapSize",	(FARPROC) WINWRAPPER_PREFIX(HeapSize),false, 0},
    {"HeapWalk",	(FARPROC) WINWRAPPER_PREFIX(HeapWalk),false, 0},
    {"HeapCreate",	(FARPROC) WINWRAPPER_PREFIX(HeapWalk),false, 0},
    {"HeapDestroy",	(FARPROC) WINWRAPPER_PREFIX(HeapWalk),false, 0}

  };



static void PatchIt (Patch *patch)
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

static void UnpatchIt (Patch *patch)
{
  //  return;
  if (patch->patched) {

    // Change rights on CRT Library module to execute/read/write.
    
    MEMORY_BASIC_INFORMATION mbi_thunk;
    VirtualQuery((void*)patch->original, &mbi_thunk, 
		 sizeof(MEMORY_BASIC_INFORMATION));
    VirtualProtect(mbi_thunk.BaseAddress, mbi_thunk.RegionSize, 
		   PAGE_EXECUTE_READWRITE, &mbi_thunk.Protect);
    
    // Restore original CRT routine.
    
    memcpy (patch->original, patch->codebytes, sizeof(X86Jump));

    // Reset CRT library code to original page protection.
    
    VirtualProtect(mbi_thunk.BaseAddress, mbi_thunk.RegionSize, 
		   mbi_thunk.Protect, &mbi_thunk.Protect);
  }
}

static bool PatchMe()
{
  bool patchedIn = false;

  // We walk through all modules loaded by the process and patch the relevant entry points.
  auto pid = GetCurrentProcessId();
  auto hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
			      PROCESS_VM_READ,
			      FALSE, pid);
  DWORD cbNeeded;
  const auto MaxModules = 8192;
  HMODULE hMods[MaxModules];
  if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
    for (auto i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
      
      TCHAR szModName[MAX_PATH] = { 0 };
      LPTSTR pszBuffer = szModName;
      if (GetModuleFileName(hMods[i], pszBuffer,
			    sizeof(szModName) / sizeof(TCHAR))) {
	HMODULE RlsCRTLibrary = GetModuleHandle(pszBuffer);

	// Patch all relevant release CRT Library entry points.
	if (RlsCRTLibrary) {
	  for (auto j = 0; j < sizeof(rls_patches) / sizeof(*rls_patches); j++) {
	    if (rls_patches[j].original = GetProcAddress(RlsCRTLibrary, rls_patches[j].import)) {
	      // Got one: patch it.
	      PatchIt(&rls_patches[j]);
	      rls_patches[j].patched = true;
	      patchedIn = true;
	    }
	  }
	}
      }
    }
  }
  return patchedIn;
}

static void UnpatchMe()
{
  for (int i = 0; i < sizeof(rls_patches) / sizeof(*rls_patches); i++) {
    //    printf ("unpatching %s\n", rls_patches[i].import);
    UnpatchIt(&rls_patches[i]);
  }
}

