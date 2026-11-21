#ifndef _RESOURCE_H
#define _RESOURCE_H

/* C linkage for the C++ consumers (libc++, LLVM, Mesa) that include this
 * header; a mangled reference would match no CactLibc export. */
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include "time.h"     /* struct timeval */
#include "unistd.h"   /* pid_t */

/* CactOS keeps no per-process resource accounting and enforces no resource
 * limits, so this header exists for the programs that ask — LLVM's file and
 * process layer reads rlimits and process times — rather than for a surface
 * CactOS implements.  getrusage() therefore reports zeros (nothing was
 * recorded), and getrlimit() reports the limits CactOS actually has: none,
 * except the thread stack, which is the size its pthreads use. */

typedef unsigned long rlim_t;

#define RLIM_INFINITY (~(rlim_t)0)

/* Resource numbers, glibc/Linux order. */
#define RLIMIT_CPU     0
#define RLIMIT_FSIZE   1
#define RLIMIT_DATA    2
#define RLIMIT_STACK   3
#define RLIMIT_CORE    4
#define RLIMIT_RSS     5
#define RLIMIT_NPROC   6
#define RLIMIT_NOFILE  7
#define RLIMIT_MEMLOCK 8
#define RLIMIT_AS      9
#define RLIMIT_LOCKS   10
#define RLIMIT_SIGPENDING 11
#define RLIMIT_MSGQUEUE   12
#define RLIMIT_NICE       13
#define RLIMIT_RTPRIO     14
#define RLIMIT_NLIMITS    15

struct rlimit {
    rlim_t rlim_cur;
    rlim_t rlim_max;
};

#define RUSAGE_SELF     0
#define RUSAGE_CHILDREN (-1)
#define RUSAGE_THREAD   1

struct rusage {
    struct timeval ru_utime;   /* user CPU time used */
    struct timeval ru_stime;   /* system CPU time used */
    long ru_maxrss;            /* maximum resident set size */
    long ru_ixrss;             /* integral shared memory size */
    long ru_idrss;             /* integral unshared data size */
    long ru_isrss;             /* integral unshared stack size */
    long ru_minflt;            /* page reclaims (soft page faults) */
    long ru_majflt;            /* page faults (hard page faults) */
    long ru_nswap;             /* swaps */
    long ru_inblock;           /* block input operations */
    long ru_oublock;           /* block output operations */
    long ru_msgsnd;            /* IPC messages sent */
    long ru_msgrcv;            /* IPC messages received */
    long ru_nsignals;          /* signals received */
    long ru_nvcsw;             /* voluntary context switches */
    long ru_nivcsw;            /* involuntary context switches */
};

int getrusage(int who, struct rusage *usage);
int getrlimit(int resource, struct rlimit *rlim);
int setrlimit(int resource, const struct rlimit *rlim);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* _RESOURCE_H */
