;/*
g++ aaa.cpp -c -o aaa.o -std=c++17 -gstabs -mcrt=clib4 -Wall -fpermissive -O3 -mstrict-align
quit
*/

#include <iostream>

#include "aaa.h"

AAA::AAA() {
}

void AAA::sayHi(const std::string &name) {
    std::cout << "Hi " << name << std::endl;
}
