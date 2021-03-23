#include <iostream>
#include <thread>
#include <unistd.h>

void bar() {
    void* p = new char[128];
    std::cerr << "p=" << p << "\n";
}

void foo() {
    bar();
}

int main() {
    foo();
    std::thread* t = new std::thread{foo};
    t->join();
    delete t;
    return 0;
}
