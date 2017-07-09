#include "pth-tmlib.h"
#include "thread.h"
#include "utility.h"
#include "tmlib-utility.h"

#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/////////////////////////////////////////////
// TM Globals
/////////////////////////////////////////////
unsigned int htm_random_seed_base = 0;
struct tm_state* gp_tm_state = NULL;
struct tm_thread* gp_tm_threads = NULL;
pthread_spinlock_t* gp_tm_flock = NULL;

void tm_init_flock() {
    gp_tm_flock = (pthread_spinlock_t*)aligned_alloc(CACHE_SIZE, sizeof(pthread_spinlock_t));
    memset((void*)gp_tm_flock, 0, sizeof(pthread_spinlock_t));

    pthread_spin_init(gp_tm_flock, PTHREAD_PROCESS_PRIVATE);
}

void tm_destroy_flock() {
    pthread_spin_destroy(gp_tm_flock);
    free((void*)gp_tm_flock);
    gp_tm_flock = NULL;
}

typedef struct tm_state {
    uint32_t padding[16];
} tm_state_t;

typedef struct tm_thread {
    uint32_t htm_random_seed;
    uint32_t myid;
    uint32_t depth;
    uint32_t padding[13];
} tm_thread_t;

void
tm_startup(size_t num_threads) {
    _Static_assert(sizeof(struct tm_state) == CACHE_SIZE, "tm_state");
    gp_tm_state = (tm_state_t*)aligned_alloc(CACHE_SIZE, sizeof(tm_state_t));
    memset(gp_tm_state, 0, sizeof(tm_state_t));

    _Static_assert(sizeof(struct tm_thread) == CACHE_SIZE, "tm_thread");
    gp_tm_threads = (tm_thread_t*)aligned_alloc(CACHE_SIZE, num_threads * sizeof(tm_thread_t));
    memset(gp_tm_threads, 0, sizeof(tm_thread_t));

    for(size_t threadId = 0; threadId < num_threads; ++threadId) {
        gp_tm_threads[threadId].myid = threadId;
        gp_tm_threads[threadId].htm_random_seed = htm_random_seed_base + threadId;
    }

    tm_init_flock();
    printf("[TMLIB] Initialized gpth\n");
}

void
tm_shutdown() {
    tm_destroy_flock();

    free(gp_tm_state);
    free(gp_tm_threads);
    gp_tm_state = NULL;
    gp_tm_threads = NULL;
}

void __attribute__ ((noinline))
tm_begin_fallback(pthread_spinlock_t* p_spinlock) {
    pthread_spin_lock(p_spinlock);
}

void __attribute__ ((noinline))
tm_end_fallback(pthread_spinlock_t* p_spinlock) {
    pthread_spin_unlock(p_spinlock);
}

void
tm_begin(long threadId, pthread_spinlock_t* p_fallback_lock) {
    tm_thread_t* p_tm_thread = gp_tm_threads + threadId;

    p_tm_thread->depth++;
    if(p_tm_thread->depth == 1) {
        tm_begin_fallback(p_fallback_lock);
    }
}

void
tm_end(long threadId, pthread_spinlock_t* p_fallback_lock) {
    tm_thread_t* p_tm_thread = gp_tm_threads + threadId;

    if(p_tm_thread->depth == 1) {
        tm_end_fallback(p_fallback_lock);
    }
    p_tm_thread->depth--;
}

#ifdef __cplusplus
}
#endif
