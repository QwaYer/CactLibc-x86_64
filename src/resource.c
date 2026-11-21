#include "resource.h"
#include "errno.h"

/* CactOS keeps no usage accounting and enforces no resource limits (see the
 * header).  What is left is the two calls a program can still be given a true
 * answer to: a zeroed rusage ("nothing was recorded") and the limits that are
 * real — none, apart from the thread stack. */

/* The size CactLibc's pthreads give a new thread; see DEFAULT_STACK_SIZE in
 * src/pthread.c.  A caller sizing a stack from RLIMIT_STACK (LLVM's
 * getDefaultStackSize) then gets the truth. */
#define CACT_THREAD_STACK_SIZE (1024ul * 1024ul)

int getrusage(int who, struct rusage *usage) {
    (void)who;
    if (!usage) return -1;
    /* No counters exist to read, so report none. */
    unsigned char *p = (unsigned char *)usage;
    for (size_t i = 0; i < sizeof(*usage); i++) p[i] = 0;
    return 0;
}

int getrlimit(int resource, struct rlimit *rlim) {
    if (!rlim) return -1;
    if (resource == RLIMIT_STACK) {
        rlim->rlim_cur = CACT_THREAD_STACK_SIZE;
        rlim->rlim_max = CACT_THREAD_STACK_SIZE;
    } else {
        /* No limit of any other kind is enforced. */
        rlim->rlim_cur = RLIM_INFINITY;
        rlim->rlim_max = RLIM_INFINITY;
    }
    return 0;
}

int setrlimit(int resource, const struct rlimit *rlim) {
    /* CactOS has no limit machinery to change. */
    (void)resource;
    (void)rlim;
    errno = ENOSYS;
    return -1;
}
