#include <heaplayers>
#include "common.hpp"
#include "sysmallocheap.hpp"


class LeakHeap : public HL::ANSIWrapper<HL::BacktraceHeap<SysMallocHeap>> {
    typedef HL::BacktraceHeap<SysMallocHeap> super;

public:
    ~LeakHeap() {
        super::print_leaks();
    }
};

static LeakHeap leakHeap;

extern "C" {
    void* malloc(size_t sz) {
        return leakHeap.malloc(sz);
    }

    void* calloc(size_t s1, size_t s2) {
        return leakHeap.calloc(s1, s2);
    }

    void* realloc(void* ptr, size_t sz) {
        return leakHeap.realloc(ptr, sz);
    }

    void free(void* ptr) {
        return leakHeap.free(ptr);
    }

    void* memalign(size_t alignment, size_t sz) {
        return leakHeap.memalign(alignment, sz);
    }
}
