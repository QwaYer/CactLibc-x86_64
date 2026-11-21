#ifndef _SCHED_H
#define _SCHED_H

/* C linkage for the C++ consumers (libc++, LLVM, Mesa) that include this
 * header; a mangled reference would match no CactLibc export. */
#ifdef __cplusplus
extern "C" {
#endif

#include "unistd.h"          /* pid_t */
#include "time.h"            /* struct timespec (sched_rr_get_interval) */

/* POSIX scheduling.
 *
 * The MLFQ has one time-sharing class, and that is what SCHED_OTHER names, so it
 * is the only supported policy.  SCHED_FIFO and SCHED_RR are defined because a
 * portable <sched.h> has to define them, but they are not supported: the
 * scheduler is a *dynamic* feedback queue and deliberately has no fixed-priority
 * class — a task's band is earned from its own run/sleep behaviour and decays
 * when it hogs the CPU, which a pinned priority would defeat — so
 * sched_setscheduler() and sched_get_priority_*() report EINVAL for them (the
 * failure POSIX specifies for a policy an implementation does not support)
 * rather than accept a priority range it could not honour.  That is also why
 * sched_priority is 0 for SCHED_OTHER, which is the only value POSIX gives it.
 *
 * Nothing here reaches the kernel except sched_yield(): every task already runs
 * in the SCHED_OTHER class, so setting that policy is not a state change.  Only
 * the calling process may be named — another pid gives EPERM, which POSIX allows
 * for a caller without permission. */
struct sched_param {
    int sched_priority;
};

#define SCHED_OTHER 0
#define SCHED_FIFO  1
#define SCHED_RR    2

/* A CPU set, as Linux spells it: a fixed bitmap of the CPUs CactOS could have.
 * Only the macros a program asks for are here. */
#define CPU_SETSIZE 1024
typedef struct { unsigned long __bits[CPU_SETSIZE / (8 * sizeof(unsigned long))]; } cpu_set_t;
#define CPU_ZERO(s)      do { for (unsigned _i = 0; _i < sizeof((s)->__bits)/sizeof((s)->__bits[0]); _i++) (s)->__bits[_i] = 0; } while (0)
#define CPU_SET(i, s)    ((s)->__bits[(i) / (8 * sizeof(unsigned long))] |= (1UL << ((i) % (8 * sizeof(unsigned long)))))
#define CPU_CLR(i, s)    ((s)->__bits[(i) / (8 * sizeof(unsigned long))] &= ~(1UL << ((i) % (8 * sizeof(unsigned long)))))
#define CPU_ISSET(i, s)  (((s)->__bits[(i) / (8 * sizeof(unsigned long))] >> ((i) % (8 * sizeof(unsigned long)))) & 1UL)

int sched_setscheduler(pid_t pid, int policy, const struct sched_param *param);
int sched_getscheduler(pid_t pid);
int sched_getparam(pid_t pid, struct sched_param *param);
int sched_setparam(pid_t pid, const struct sched_param *param);
int sched_get_priority_max(int policy);
int sched_get_priority_min(int policy);
int sched_rr_get_interval(pid_t pid, struct timespec *interval);
int sched_yield(void);

/* The CPU the caller is running on.  CactOS's kernel reports no per-task CPU
 * id to userspace, so this is a fixed 0 — good enough for the callers that use
 * it as a locality hint. */
int sched_getcpu(void);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
