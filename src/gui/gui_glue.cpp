/*
https://stackoverflow.com/questions/2744181/how-to-call-c-function-from-c
g++ gui_glue.cpp -c -o gui_glue.o -std=c++17 -gstabs -mcrt=clib4 -Wall -fpermissive -O3 -mstrict-align
*/


//#include <cstdlib>

#include "gui_glue.h"
#include "aaa.h"


#ifdef __cplusplus
extern "C" {
#endif

// Inside this "extern C" block, I can implement functions in C++, which will externally 
//   appear as C functions (which means that the function IDs will be their names, unlike
//   the regular C++ behavior, which allows defining multiple functions with the same name
//   (overloading) and hence uses function signature hashing to enforce unique IDs),


static AAA *AAA_instance = NULL;

void lazyAAA() {
    if (AAA_instance == NULL) {
        AAA_instance = new AAA();
    }
}

void AAA_sayHi(const char *name) {
    lazyAAA();
    AAA_instance->sayHi(name);
}

#ifdef __cplusplus
}
#endif
