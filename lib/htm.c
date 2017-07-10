#include "htm.h"

#ifdef __cplusplus
extern "C" {
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter" 
#pragma GCC diagnostic ignored "-Wreturn-type"

/////////////////////////////////////////////
// HTM Intrinsics
/////////////////////////////////////////////
int __attribute__ ((noinline))
htm_start(int arg, int arg2) {
    __asm__ __volatile__ (".word 0x70000000":::"memory");
}

int __attribute__ ((noinline))
htm_meta(int arg, int arg2) {
    __asm__ __volatile__ (".word 0x74000000":::"memory");
}

void __attribute__ ((noinline))
htm_commit(int arg, int arg2) {
    __asm__ __volatile__ (".word 0x78000000":::"memory");
}

#pragma GCC diagnostic pop

#ifdef __cplusplus
}
#endif
