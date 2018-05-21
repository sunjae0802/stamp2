#ifndef TM_H
#define TM_H 1

#include <stdlib.h>                   /* Defines size_t. */
#include "pth-tmlib.h"

#define TM_STARTUP(numThread)         tm_startup(numThread)
#define TM_SHUTDOWN()                 tm_shutdown()

#define MEMORY_STARTUP(numThread)     memory_init(numThread, 16*1024*1024, 0);
#define MEMORY_SHUTDOWN()             memory_destroy()

#define TM_BEGIN(_threadId)           tm_begin(_threadId, gp_tm_flock)
#define TM_END(_threadId)             tm_end(_threadId, gp_tm_flock)
#define TM_RESTART()                  tm_restart()

#define TM_PURE                       /* nothing */
#define TM_SAFE                       /* nothing */

#define MALLOC(size)                  memory_get(thread_getId(), size)
#define FREE(ptr)                     /* nothing */

#define TM_SHARED_READ(var)           var
#define TM_SHARED_READ_P(var)         var
#define TM_SHARED_READ_F(var)         var

#define TM_SHARED_WRITE(var, val)     var = val
#define TM_SHARED_WRITE_P(var, val)   var = val
#define TM_SHARED_WRITE_F(var, val)   var = val

#define TM_LOCAL_WRITE(var, val)      var = val
#define TM_LOCAL_WRITE_P(var, val)    var = val
#define TM_LOCAL_WRITE_F(var, val)    var = val

/* Indirect function call management */
/* In STAMP applications, it is safe to use transaction_pure */
//#define TM_IFUNC_DECL                 __attribute__((transaction_pure))
#define TM_IFUNC_CALL1(r, f, a1)      r = f(a1)
#define TM_IFUNC_CALL2(r, f, a1, a2)  r = f((a1), (a2))

/* Additional annotations */
/* strncmp can be defined as a macro (FORTIFY_LEVEL) */
#ifdef strncmp
# undef strncmp
#endif
extern
TM_PURE
int strncmp (__const char *__s1, __const char *__s2, size_t __n);

extern
TM_PURE
void __assert_fail (__const char *__assertion, __const char *__file,
                           unsigned int __line, __const char *__function)
     __attribute__ ((__noreturn__));


#endif /* TM_H */

