#ifndef _PTHREAD_H
#define _PTHREAD_H

/* C linkage for the C++ consumers (libc++, LLVM, Mesa) that include this
 * header; a mangled reference would match no CactLibc export. */
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <time.h>
#include "sched.h"           /* struct sched_param, SCHED_OTHER */
#include "signal.h"          /* sigset_t (pthread_sigmask) */

/*
 * POSIX threads for CactOS.
 *
 * The Cact kernel now provides user threads (CACT_PROCCTL_THREAD_CREATE),
 * futex-style wait/wake (CACT_PROCCTL_FUTEX) and a kernel-cleared join word
 * (clear_child_tid).  On top of those this header offers the subset libc
 * consumers need: mutexes, condition variables, once, per-thread keys, and
 * pthread_create/join/exit.
 */

typedef uint32_t pthread_t;

/* `state` is the futex word: 0 = unlocked, 1 = locked (no waiters),
 * 2 = locked with at least one waiter. */
typedef struct {
    volatile int state;
    int          type;
    int          count;   /* recursion depth (recursive mutexes) */
    int          owner;   /* owning tid (recursive/errorcheck) */
} pthread_mutex_t;

/* `seq` is the futex word: bumped by signal/broadcast, waited on by waiters. */
typedef struct {
    volatile int seq;
} pthread_cond_t;

typedef struct {
    volatile int done;    /* 0 = unstarted, 1 = running, 2 = done */
} pthread_once_t;

typedef uint32_t pthread_key_t;

typedef struct { uint32_t __x; } pthread_mutexattr_t;
/* The condition-variable attribute carries one thing: which clock
 * pthread_cond_timedwait() reads its absolute timeout against (CLOCK_REALTIME
 * by default, CLOCK_MONOTONIC on request).  pthread_cond_init() records that
 * choice in the condition itself.  The word keeps its size, so a consumer built
 * against the old header is undisturbed. */
typedef struct { uint32_t clock; } pthread_condattr_t;

/* The scheduling half of a thread attribute.  POSIX starts a new thread with
 * INHERIT_SCHED, so policy/param below only take effect once
 * pthread_attr_setinheritsched(attr, PTHREAD_EXPLICIT_SCHED) is called — and
 * then only SCHED_OTHER (priority 0) can be honoured, because that is the one
 * class the MLFQ has; pthread_create() reports EINVAL for anything else rather
 * than silently starting the thread in the wrong class (see <sched.h>). */
typedef struct {
    int                policy;
    int                inheritsched;
    struct sched_param param;
} pthread_attr_t;

/* A rendezvous barrier built from one mutex and one condition: `trip` threads
 * arrive, the last one bumps the generation and wakes the rest, and every
 * waiter leaves only once the generation has moved past the one it arrived in
 * (which is what keeps a fast thread from slipping into the next round early). */
typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
    unsigned        trip;        /* threads per generation */
    unsigned        arrived;
    unsigned        generation;
} pthread_barrier_t;

typedef struct { uint32_t __x; } pthread_barrierattr_t;

/* A readers-writer lock over one mutex + one condition: `readers` counts active
 * readers (the lock is held for reading while it is > 0), `writer` is 1 while a
 * writer holds it, and `waiting_writers` makes a late reader wait while a
 * writer is queued so writers cannot starve. */
typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
    int             readers;
    int             writer;
    int             waiting_writers;
} pthread_rwlock_t;

typedef struct { uint32_t __x; } pthread_rwlockattr_t;

#define PTHREAD_MUTEX_INITIALIZER  { 0, 0, 0, 0 }
#define PTHREAD_COND_INITIALIZER   { 0 }
#define PTHREAD_ONCE_INIT          { 0 }
#define PTHREAD_BARRIER_INITIALIZER(n) \
    { PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, (n), 0, 0 }
#define PTHREAD_RWLOCK_INITIALIZER \
    { PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, 0, 0, 0 }

#define PTHREAD_MUTEX_NORMAL     0
#define PTHREAD_MUTEX_RECURSIVE  1
#define PTHREAD_MUTEX_ERRORCHECK 2
#define PTHREAD_MUTEX_DEFAULT    PTHREAD_MUTEX_NORMAL

#define PTHREAD_KEYS_MAX 64
#define PTHREAD_DESTRUCTOR_ITERATIONS 4

#define PTHREAD_CREATE_JOINABLE 0
#define PTHREAD_CREATE_DETACHED 1

#define PTHREAD_INHERIT_SCHED   0
#define PTHREAD_EXPLICIT_SCHED  1

#define PTHREAD_SCOPE_SYSTEM    0   /* kernel threads: the only scope there is */
#define PTHREAD_SCOPE_PROCESS   1

int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
int pthread_mutex_destroy(pthread_mutex_t *mutex);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_trylock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);

/* The attribute object carries one thing — the mutex type — in its single
 * word, and pthread_mutex_init() reads it (a NULL attr means
 * PTHREAD_MUTEX_NORMAL).  libc++ builds its std::recursive_mutex this way. */
int pthread_mutexattr_init(pthread_mutexattr_t *attr);
int pthread_mutexattr_destroy(pthread_mutexattr_t *attr);
int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type);
int pthread_mutexattr_gettype(const pthread_mutexattr_t *attr, int *type);

int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr);
int pthread_cond_destroy(pthread_cond_t *cond);
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);
int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex,
                           const struct timespec *abstime);
int pthread_cond_signal(pthread_cond_t *cond);
int pthread_cond_broadcast(pthread_cond_t *cond);

int pthread_once(pthread_once_t *once_control, void (*init_routine)(void));

int pthread_key_create(pthread_key_t *key, void (*destructor)(void *));
int pthread_key_delete(pthread_key_t key);
void *pthread_getspecific(pthread_key_t key);
int pthread_setspecific(pthread_key_t key, const void *value);

pthread_t pthread_self(void);
int pthread_equal(pthread_t t1, pthread_t t2);

int pthread_attr_init(pthread_attr_t *attr);
int pthread_attr_destroy(pthread_attr_t *attr);
int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                   void *(*start_routine)(void *), void *arg);
void pthread_exit(void *retval) __attribute__((noreturn));
int pthread_join(pthread_t thread, void **retval);
int pthread_detach(pthread_t thread);

/* The scheduling attributes (see the comment on pthread_attr_t). */
int pthread_attr_setinheritsched(pthread_attr_t *attr, int inheritsched);
int pthread_attr_getinheritsched(const pthread_attr_t *attr, int *inheritsched);
int pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy);
int pthread_attr_getschedpolicy(const pthread_attr_t *attr, int *policy);
int pthread_attr_setschedparam(pthread_attr_t *attr, const struct sched_param *param);
int pthread_attr_getschedparam(const pthread_attr_t *attr, struct sched_param *param);
int pthread_attr_setscope(pthread_attr_t *attr, int scope);
int pthread_attr_getscope(const pthread_attr_t *attr, int *scope);

int pthread_setschedparam(pthread_t thread, int policy, const struct sched_param *param);
int pthread_getschedparam(pthread_t thread, int *policy, struct sched_param *param);
int pthread_setschedprio(pthread_t thread, int prio);

/* Condition-variable attributes (see the typedef above): init/destroy and the
 * clock choice.  Only CLOCK_REALTIME and CLOCK_MONOTONIC are accepted — the two
 * clocks CactOS has. */
int pthread_condattr_init(pthread_condattr_t *attr);
int pthread_condattr_destroy(pthread_condattr_t *attr);
int pthread_condattr_setclock(pthread_condattr_t *attr, int clock_id);
int pthread_condattr_getclock(const pthread_condattr_t *attr, int *clock_id);

/* Barriers: `count` threads rendezvous per generation; exactly one of them gets
 * PTHREAD_BARRIER_SERIAL_THREAD, the rest 0. */
#define PTHREAD_BARRIER_SERIAL_THREAD (-1)
int pthread_barrierattr_init(pthread_barrierattr_t *attr);
int pthread_barrierattr_destroy(pthread_barrierattr_t *attr);
int pthread_barrier_init(pthread_barrier_t *barrier, const pthread_barrierattr_t *attr, unsigned count);
int pthread_barrier_destroy(pthread_barrier_t *barrier);
int pthread_barrier_wait(pthread_barrier_t *barrier);

/* Readers-writer locks (see the typedef above). */
int pthread_rwlockattr_init(pthread_rwlockattr_t *attr);
int pthread_rwlockattr_destroy(pthread_rwlockattr_t *attr);
int pthread_rwlock_init(pthread_rwlock_t *rwlock, const pthread_rwlockattr_t *attr);
int pthread_rwlock_destroy(pthread_rwlock_t *rwlock);
int pthread_rwlock_rdlock(pthread_rwlock_t *rwlock);
int pthread_rwlock_tryrdlock(pthread_rwlock_t *rwlock);
int pthread_rwlock_wrlock(pthread_rwlock_t *rwlock);
int pthread_rwlock_trywrlock(pthread_rwlock_t *rwlock);
int pthread_rwlock_unlock(pthread_rwlock_t *rwlock);

/* Signal mask of the *calling thread*.  CactOS's signals are per-process (the
 * kernel carries one mask per task and delivers process-wide), so this is
 * sigprocmask() — which is what the caller's intent amounts to here. */
int pthread_sigmask(int how, const sigset_t *set, sigset_t *oldset);

/* Name the calling thread.  The kernel keeps no thread names, so this is
 * accepted and does nothing; a length ERANGE, as glibc reports it, is not
 * produced here (there is no stored name to overflow). */
int pthread_setname_np(pthread_t thread, const char *name);

/* A clock that reads the thread's CPU time.  CactOS records no per-thread CPU
 * accounting, so the monotonic clock stands in: the caller gets a real,
 * monotone value rather than an error or an uninitialised one. */
int pthread_getcpuclockid(pthread_t thread, clockid_t *clock_id);

/* fork() handlers.  Called in POSIX's order: every prepare in reverse
 * registration order, then every parent/child in order. */
int pthread_atfork(void (*prepare)(void), void (*parent)(void), void (*child)(void));

/* Internal: the hooks fork() (src/unistd.c) calls around the trap. */
void __cact_atfork_prepare(void);
void __cact_atfork_parent(void);
void __cact_atfork_child(void);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* _PTHREAD_H */
