#include "pth-tmlib.h"
#include "thread.h"
#include "utility.h"
#include "tmlib-utility.h"
#include "htm.h"

#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

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
struct timeval g_start_tv;
#define TMLIB_ABORT_THRESHOLD         (12)

enum TMWAIT_TYPE {
    TMWAIT_ACTIVE_FALLBACK = 0,
    TMWAIT_BACKOFF = 1
};

typedef struct tm_state {
    uint32_t padding[16];
} tm_state_t;

typedef struct tm_thread {
    uint32_t htm_random_seed;
    uint32_t myid;
    uint32_t depth;
    uint32_t in_fallback;
    uint32_t num_aborts;
    uint32_t padding[11];
} tm_thread_t;

void
tm_startup(size_t num_threads) {
    _Static_assert(sizeof(struct tm_state) == CACHE_SIZE, "tm_state");
    gp_tm_state = (tm_state_t*)aligned_alloc(CACHE_SIZE, sizeof(tm_state_t));
    memset(gp_tm_state, 0, sizeof(tm_state_t));

    _Static_assert(sizeof(struct tm_thread) == CACHE_SIZE, "tm_thread");
    gp_tm_threads = (tm_thread_t*)aligned_alloc(CACHE_SIZE, num_threads * sizeof(tm_thread_t));
    memset(gp_tm_threads, 0, num_threads * sizeof(tm_thread_t));

    for(size_t threadId = 0; threadId < num_threads; ++threadId) {
        gp_tm_threads[threadId].myid = threadId;
        gp_tm_threads[threadId].htm_random_seed = htm_random_seed_base + threadId;
    }

    printf("[TMLIB] Initialized htm-hwlock-lin (%d)\n", TMLIB_ABORT_THRESHOLD);
    gettimeofday(&g_start_tv, NULL);
}

void
tm_shutdown() {
    struct timeval end_tv;
    gettimeofday(&end_tv, NULL);

    struct timeval roi;
    timersub(&end_tv, &g_start_tv, &roi);

    int64_t roi_usecs = roi.tv_sec * 1000000LL + roi.tv_usec;
    printf("[TMLIB] Ending ROI: %lld\n", roi_usecs);

    free(gp_tm_state);
    free(gp_tm_threads);
    gp_tm_state = NULL;
    gp_tm_threads = NULL;
}

void __attribute__ ((noinline))
tm_begin_hwl_fb(tm_thread_t* p_tm_thread) {
    while(htm_start(1, 0)) {
        tm_random_delay(p_tm_thread->myid);
    }
}

void __attribute__ ((noinline))
tm_end_hwl_fb() {
    htm_commit(1, 0);
}

void __attribute__ ((noinline))
tm_wait(uint32_t arg, tm_thread_t* p_tm_thread) {
    if(arg == TMWAIT_BACKOFF) {
        DELAY(p_tm_thread->num_aborts, &(p_tm_thread->htm_random_seed), 64);
    } else {
        tm_random_delay(p_tm_thread->myid);
    }
}

void
tm_begin(long threadId, pthread_spinlock_t* p_fallback_lock __attribute__((unused)) ) {
    tm_thread_t* p_tm_thread = gp_tm_threads + threadId;

    p_tm_thread->depth++;
    if(p_tm_thread->depth == 1) {
        p_tm_thread->num_aborts = 0;
        int retry = 0;
        do {
            retry = 0;

            int ret = htm_start(0, threadId);

            if(ret == 0) {
                // HTM has been started
            } else {
                if(htm_begin_nacked(ret)) {
                    tm_wait(TMWAIT_ACTIVE_FALLBACK, p_tm_thread);
                    retry = 1;
                } else if(htm_is_syscall(ret) || htm_capacity_abort(ret)) {
                    p_tm_thread->in_fallback = 1;
                    tm_begin_hwl_fb(p_tm_thread);
                } else {
                    // Other abort path
                    p_tm_thread->num_aborts++;
                    tm_wait(TMWAIT_BACKOFF, p_tm_thread);

                    if(p_tm_thread->num_aborts > TMLIB_ABORT_THRESHOLD) {
                        p_tm_thread->in_fallback = 1;
                        tm_begin_hwl_fb(p_tm_thread);
                        // Since retry is 0, we will exit
                    } else {
                        retry = 1;
                    }
                }
            }
        } while(retry);
    }
}

void
tm_end(long threadId, pthread_spinlock_t* p_fallback_lock __attribute__((unused)) ) {
    tm_thread_t* p_tm_thread = gp_tm_threads + threadId;

    if(p_tm_thread->depth == 1) {
        if(p_tm_thread->in_fallback) {
            tm_end_hwl_fb();
            p_tm_thread->in_fallback = 0;
        } else {
            htm_commit(0, 0);
        }
    }
    p_tm_thread->depth--;
}

void
tm_random_delay(long threadId) {
    tm_thread_t* p_tm_thread = gp_tm_threads + threadId;
    DELAY(16, &(p_tm_thread->htm_random_seed), 2);
}

#ifdef __cplusplus
}
#endif
