#ifndef HTM_H
#define HTM_H 1

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/////////////////////////////////////////////
// HTM Intrinsics
/////////////////////////////////////////////
int __attribute__ ((noinline))
htm_start(int arg, int arg2);

int __attribute__ ((noinline))
htm_meta(int arg, int arg2);

void __attribute__ ((noinline))
htm_commit(int arg, int arg2);

static inline int
htm_abort(int arg) {
    return htm_meta(100, arg);
}

static inline int
htm_test() {
    return htm_meta(0, 0);
}

int tmlib_interrupt(int arg1, unsigned int arg2);

/////////////////////////////////////////////
// HTM Sstart return value tests
/////////////////////////////////////////////
//       PID: 7-5
//       ARG: 4
//       abortArg: 
//       isAbort: 0
// ret = 7654 3210
static inline uint32_t
htm_get_aborter_pid(uint32_t ret) {
    return (ret & 0xFFF000) >> 12;
}

static inline uint32_t
htm_get_abort_arg(uint32_t ret) {
    return (ret & 0xF00) >> 8;
}

#define HTM_RV_ATYPE_MASK   (0xFF)
#define HTM_RV_IS_ABORT     (1)
enum htm_atype {
    HTM_ATYPE_SYSCALL   = 2,
    HTM_ATYPE_USER      = 4,
    HTM_ATYPE_CAPACITY  = 8,
};

static inline uint32_t
htm_is_syscall(uint32_t ret) {
    return (ret & HTM_RV_ATYPE_MASK) == (HTM_RV_IS_ABORT | HTM_ATYPE_SYSCALL);
}

static inline uint32_t
htm_is_user(uint32_t ret) {
    return (ret & HTM_RV_ATYPE_MASK) == (HTM_RV_IS_ABORT | HTM_ATYPE_USER);
}

static inline uint32_t
htm_capacity_abort(uint32_t ret) {
    return (ret & HTM_RV_ATYPE_MASK) == (HTM_RV_IS_ABORT | HTM_ATYPE_CAPACITY);
}

static inline uint32_t
htm_begin_nacked(uint32_t ret) {
    return (ret & 0xFF) == 2;
}

static inline unsigned int INTR_TYPE_MASK(unsigned int arg1) {
    return arg1 & (0xFF);
}
static inline unsigned int INTR_TID_MASK(unsigned int arg1) {
    return arg1 >> 8;
}

#ifdef __cplusplus
}
#endif

#endif /* HTM_H */

/* =============================================================================
 *
 * End of htm.h
 *
 * =============================================================================
 */
