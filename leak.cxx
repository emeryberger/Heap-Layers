#include <heaplayers>
#include "common.hpp"
#include "sysmallocheap.hpp"
#include "heapredirect.hpp"


class LeakHeap : public HL::ANSIWrapper<HL::BacktraceHeap<SysMallocHeap>> {
    typedef HL::BacktraceHeap<SysMallocHeap> super;

public:
    void lock() {}
    void unlock() {}
};

HEAP_REDIRECT(LeakHeap, 8 * 1024 * 1024);

class LeaksPrinter {
public:
    LeaksPrinter() {
        // ignore allocations from program initialization (before this object's constructor)
        Wrapper::getTheCustomHeap().clear_leaks();
    }

    ~LeaksPrinter() {
        Wrapper::getTheCustomHeap().print_leaks();
    }

} printer;
