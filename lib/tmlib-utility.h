#ifndef TMLIB_UTILITY_H
#define TMLIB_UTILITY_H 1

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/////////////////////////////////////////////
// Busy-wait Delay Function
/////////////////////////////////////////////

static inline int
busywait(int c)
{
    if(c == 0) return 0;

    int i = c;
    while(--c) {
        i = c * 2;
        __asm__ __volatile__("");
    }
    return i;
}

/////////////////////////////////////////////
// Force Alignment
/////////////////////////////////////////////

#define CACHE_SIZE 64

static inline void*
align_addr(void* pointer, unsigned int align) {
#if __WORDSIZE == 64
    uint64_t p = (uint64_t)pointer;
    uint64_t offset = p % align;
#else
    uint32_t p = (uint32_t)pointer;
    uint32_t offset = p % align;
#endif
    if(offset == 0) {
        return pointer;
    } else {
        return (void*)(p + (align - offset));
    }
}

#define DELAY(_arg, _pseed, _scale) do {                    \
            int r = rand_r(_pseed);                         \
            int delay = ((r % (_arg)) + 1);                 \
            busywait(delay * (_scale));                     \
        } while(0);


#ifdef __cplusplus
}
#endif

#endif /* TMLIB_UTILITY_H */

/* =============================================================================
 *
 * End of tmlib-utility.h
 *
 * =============================================================================
 */
