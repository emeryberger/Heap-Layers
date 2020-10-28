/*

 Generic replacement for memalign in terms of xxmalloc and xxfree.

 LIMITATIONS:

 - This code assumes that the underlying allocator will do "the right
   thing" when xxfree() is called with a pointer internal to an
   allocated object. Header-based allocators, for example, need not
   apply.

*/

extern "C" {
  void * xxmalloc(size_t);
  void xxfree(void *);
}

extern "C"
void * generic_xxmemalign(size_t alignment, size_t size) {
  // Check for non power-of-two alignment.
  if ((alignment == 0) || (alignment & (alignment - 1)))
    {
      return nullptr;
    }

  if (alignment <= alignof(max_align_t)) {
    // Already aligned by default.
    return xxmalloc(size);
  } else {
    // Try to just allocate an object of the requested size.
    // If it happens to be aligned properly, just return it.
    void * ptr = xxmalloc(size);
    if (((size_t) ptr & ~(alignment - 1)) == (size_t) ptr) {
      // It is already aligned just fine; return it.
      return ptr;
    }
    // It was not aligned as requested: free the object and allocate a big one,
    // and align within.
    xxfree(ptr);
    ptr = xxmalloc (size + 2 * alignment);
    void * alignedPtr = (void *) (((size_t) ptr + alignment - 1) & ~(alignment - 1));
    return alignedPtr;
  }
}
