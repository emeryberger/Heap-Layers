# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Heap Layers is a C++ template-based infrastructure for composing high-performance memory allocators using the "mixin layers" pattern. It's a **header-only library** used to build custom allocators like Hoard and DieHard.

## Build Commands

### Using the Library
Include `heaplayers.h` in your project and add the repository to your include path:
```bash
g++ --std=c++14 -I/path/to/Heap-Layers your_allocator.cpp
```

### Building the Kingsley Example (malloc replacement)
```bash
cd examples/kingsley
mkdir -p build && cd build
cmake ..
make
```

Or using the shell script:
```bash
cd examples/kingsley
./compile
```

### Running with a Custom Allocator
On Linux:
```bash
LD_PRELOAD=./libkingsley.so your_program
```

On macOS:
```bash
DYLD_INSERT_LIBRARIES=./libkingsley.dylib your_program
```

## Architecture

### Mixin Layer Pattern
Heap layers compose via C++ template inheritance. Each layer wraps a `SuperHeap` and adds/modifies behavior:

```cpp
template <class SuperHeap>
class MyLayer : public SuperHeap {
  void* malloc(size_t sz) {
    // Pre-processing
    void* ptr = SuperHeap::malloc(sz);
    // Post-processing
    return ptr;
  }
};
```

### Heap Layer Categories (`heaps/`)

| Directory | Purpose |
|-----------|---------|
| `top/` | Source heaps with no superheap (`MallocHeap`, `MmapHeap`) |
| `buildingblock/` | Core components (`FreelistHeap`, `ChunkHeap`, `AdaptHeap`) |
| `objectrep/` | Add metadata to objects (`SizeHeap`, `AddHeap`) |
| `combining/` | Combine multiple heaps (`SegHeap`, `StrictSegHeap`, `HybridHeap`) |
| `threads/` | Thread safety (`LockedHeap`, `ThreadSpecificHeap`) |
| `special/` | Special allocation patterns (`ZoneHeap`, `BumpAlloc`, `ObstackHeap`) |
| `debug/` | Debugging/stats (`DebugHeap`, `StatsHeap`, `LogHeap`) |
| `utility/` | Utilities (`UniqueHeap`, `ExactlyOneHeap`, `NullHeap`) |
| `general/` | Complete allocators (`KingsleyHeap`, `DLHeap`) |

### Key Source Heaps
- `MmapHeap` - Gets memory from OS via mmap/VirtualAlloc
- `MallocHeap` - Wraps system malloc (for testing/comparison)

### Creating a Custom Allocator

1. Define your heap type by composing layers:
```cpp
class TopHeap : public SizeHeap<UniqueHeap<ZoneHeap<MmapHeap, 65536>>> {};

class MyAllocator :
  public ANSIWrapper<KingsleyHeap<AdaptHeap<DLList, TopHeap>, TopHeap>> {};
```

2. Implement the `xx*` interface in a `.cpp` file:
```cpp
extern "C" {
  void* xxmalloc(size_t sz);      // Required
  void xxfree(void* ptr);         // Required
  void* xxmemalign(size_t, size_t); // Required
  size_t xxmalloc_usable_size(void*); // Required
  void xxmalloc_lock();           // For fork safety
  void xxmalloc_unlock();         // For fork safety
}
```

3. Link with the appropriate wrapper:
   - Linux: `wrappers/gnuwrapper.cpp`
   - macOS: `wrappers/macwrapper.cpp`
   - Windows: `wrappers/winwrapper.cpp`

### Common Layer Compositions

- `SizeHeap<X>` - Adds size header to track allocation sizes
- `UniqueHeap<X>` - Ensures single instance of underlying heap
- `ZoneHeap<X, ChunkSize>` - Arena allocator, batches frees
- `LockedHeap<LockType, X>` - Adds thread safety
- `ANSIWrapper<X>` - ANSI C malloc/free/realloc semantics
- `AdaptHeap<Dict, X>` - Maintains freelist via dictionary structure
- `KingsleyHeap<PerClass, Big>` - Power-of-two segregated fits

### Required Heap Interface

Each layer should expose:
- `void* malloc(size_t sz)`
- `void free(void* ptr)`
- `size_t getSize(void* ptr)` - for layers that track size
- `enum { Alignment = N }` - alignment guarantee

## Key Files

- `heaplayers.h` - Master include file
- `wrappers/wrapper.cpp` - Implements malloc/free/new/delete from xx* functions
- `wrappers/ansiwrapper.h` - ANSI C semantics (NULL handling, alignment)
- `examples/kingsley/libkingsley.cpp` - Complete example allocator
