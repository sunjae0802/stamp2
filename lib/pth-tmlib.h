#ifndef PTH_TMLIB_H
#define PTH_TMLIB_H 1

#include <stdint.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

extern unsigned int htm_random_seed_base;
extern pthread_spinlock_t* gp_tm_flock;

void
tm_startup(size_t num_threads);

void
tm_shutdown();

void
tm_begin(long threadId, pthread_spinlock_t* p_fallback_lock);

void
tm_end(long threadId, pthread_spinlock_t* p_fallback_lock);

void
tm_random_delay(long threadId);

static inline int
pthread_spin_test(pthread_spinlock_t* p_spinlock) {
    volatile uint32_t* p_spin = (volatile uint32_t*)(p_spinlock);
    int result = (*p_spin) == 1;
    __sync_synchronize();
    return result;
}

#ifdef __cplusplus
}
#endif


#endif /* PTH_TMLIB_H */


/* =============================================================================
 *
 * End of pth_tmlib.h
 *
 * =============================================================================
 */
