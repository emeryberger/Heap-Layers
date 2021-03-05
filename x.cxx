#include <iostream>
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
    return 0;
}
