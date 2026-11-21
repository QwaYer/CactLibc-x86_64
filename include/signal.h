#ifndef _SIGNAL_H
#define _SIGNAL_H

/* C linkage for the C++ consumers (libc++, LLVM, Mesa) that include this
 * header; a mangled reference would match no CactLibc export. */
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include "time.h"
#include "unistd.h"

typedef uint32_t sigset_t;

/* how values for sigprocmask */
#define SIG_BLOCK   0
#define SIG_UNBLOCK 1
#define SIG_SETMASK 2

/*
 * Signal bitmasks — each signal occupies one bit.
 * These values are used with sigprocmask/sigpending/sigsuspend
 * and match the kernel's internal representation.
 */
#define SIGKILL  (1u << 0)
#define SIGTERM  (1u << 1)
#define SIGSTOP  (1u << 2)
#define SIGCONT  (1u << 3)
#define SIGPIPE  (1u << 4)
#define SIGALRM  (1u << 5)
#define SIGCHLD  (1u << 6)
#define SIGFPE   (1u << 7)
#define SIGSEGV  (1u << 8)
#define SIGWINCH (1u << 9)
#define SIGHUP   (1u << 10)   /* hangup — terminal closed          */
#define SIGINT   (1u << 11)   /* interrupt — Ctrl-C from TTY       */
#define SIGQUIT  (1u << 12)   /* quit     — Ctrl-\ from TTY        */

/* The rest of POSIX's fault/job-control set.  Each is still one mask bit, like
 * every other SIG* here: the kernel's signal tables are indexed by the bit (bit
 * 0 is SIGKILL, and NSIG/KERNEL_NSIG is the number of slots).  Before
 * 2026-11-21 these six were POSIX *numbers* (13/14/15/20/21/22), and those
 * numbers also drive the POSIX-number arm of sig_number_to_sigindex()
 * (src/signal.c): SIGILL (13) resolved to SIGPIPE's slot, SIGABRT (14) to
 * SIGALRM's and SIGBUS (15) to SIGTERM's — so abort() raised SIGALRM's slot and
 * LLVM's crash handler clobbered SIGPIPE/SIGALRM/SIGTERM.  As bits there is no
 * such collision. */
#define SIGILL   (1u << 13)
#define SIGTRAP  (1u << 14)
#define SIGABRT  (1u << 15)
#define SIGBUS   (1u << 16)
#define SIGUSR1  (1u << 17)
#define SIGUSR2  (1u << 18)

/* ioctl requests for terminal window size */
#define TIOCGWINSZ 0x5413
#define TIOCSWINSZ 0x5414

struct winsize {
    uint16_t ws_row;
    uint16_t ws_col;
    uint16_t ws_xpixel;
    uint16_t ws_ypixel;
};

/* itimer types for setitimer/getitimer */
#define ITIMER_REAL    0
#define ITIMER_VIRTUAL 1
#define ITIMER_PROF    2

struct itimerval {
    struct timeval it_interval; /* interval for periodic timer */
    struct timeval it_value;    /* time until next expiry      */
};

typedef void (*sighandler_t)(int);

#define SIG_DFL ((sighandler_t)0)
#define SIG_IGN ((sighandler_t)1)
#define SIG_ERR ((sighandler_t)(uintptr_t)-1)

#define WNOHANG 1

/* SA_SIGINFO: sigaction flag for sa_sigaction handler.
 * The kernel builds the siginfo_t and ucontext_t into the signal frame, so the
 * handler is entered as void handler(int, siginfo_t *, void *).  si_code is
 * SI_USER for a sent signal and SI_KERNEL for one the kernel generated (a
 * fault), and si_addr is then the address that faulted. */
#define SA_SIGINFO 0x0004
#define SA_NODEFER 0x40000000
/* Run this handler on the alternate signal stack (see sigaltstack below).
 * The value matches CACT_SA_ONSTACK in ioctl_abi.h. */
#define SA_ONSTACK 0x08000000
/* POSIX's "reset this signal's disposition to SIG_DFL when the handler is
 * entered".  CactOS's kernel does not implement it — it acts on SA_ONSTACK,
 * SA_SIGINFO and SA_NODEFER and stores the rest unread — so the flag is
 * accepted and does nothing.  LLVM's signal handler does not rely on it: it
 * unregisters every handler itself on entry. */
#define SA_RESETHAND 0x80000000

/* FPE signal code constants */
#define FPE_INTDIV 1
#define FPE_FLTDIV 2

/* sigqueue()'s payload: POSIX writes it as `union sigval`, and the kernel
 * carries it (as one word) with the pending signal, so both a sigwaitinfo() and
 * an SA_SIGINFO handler read it back as si_value. */
union sigval {
    int   sival_int;
    void *sival_ptr;
};

typedef struct {
    int   si_signo;
    int   si_code;
    void *si_addr;
    union sigval si_value;
} siginfo_t;

/* The kernel builds this into the signal frame itself (`cact_siginfo_t` in
 * Cact/kernel/proc/task.h) and enters the handler with a pointer to it, so the
 * two definitions have to stay one shape: two ints, then two pointer-sized
 * fields.  On x86_64 that is 4 + 4 + 8 + 8. */
_Static_assert(sizeof(siginfo_t) == 2 * sizeof(int) + 2 * sizeof(void *),
               "siginfo_t layout drift");

/* ── sigevent ─────────────────────────────────────────────────────────────
 * POSIX's event-delivery descriptor, used by mq_notify(2) (and, in time, the
 * *_async() family).  Only SIGEV_SIGNAL and SIGEV_NONE are honoured: a
 * SIGEV_SIGNAL notification delivers sigev_signo to the process, and
 * SIGEV_THREAD is refused with EINVAL.  The layout matches glibc, so a
 * caller compiled against either header passes the same thing. */
#define SIGEV_SIGNAL 0      /* deliver sigev_signo to the process */
#define SIGEV_NONE   1      /* do not notify */
#define SIGEV_THREAD 2      /* not supported on this kernel */

struct sigevent {
    union sigval sigev_value;
    int          sigev_signo;
    int          sigev_notify;
    union {
        int _tid;                                  /* SIGEV_THREAD_ID (unused) */
        struct {
            void (*_function)(union sigval);       /* SIGEV_THREAD (unused)   */
            void  *_attribute;
        } _sigev_thread;
    } _sigev_un;
};

typedef struct sigevent sigevent_t;

/* ── alternate signal stack ───────────────────────────────────────────────
 * A stack the kernel switches to for handlers installed with SA_ONSTACK — the
 * way a SIGSEGV from stack overflow gets a chance to report anything.  The
 * kernel carries the state per task and disables it across exec(). */
typedef struct {
    void  *ss_sp;
    int    ss_flags;
    size_t ss_size;
} stack_t;

#define SS_ONSTACK 1        /* a handler is running on it right now */
#define SS_DISABLE 2        /* not in use */
#define MINSIGSTKSZ 2048    /* matches CACT_MINSIGSTKSZ */

int sigaltstack(const stack_t *ss, stack_t *old_ss);

#define sigemptyset(set)    (*(set) = 0)

/* Every signal a process can block — all of them except the two that cannot be
 * caught (SIGKILL, SIGSTOP). */
int sigfillset(sigset_t *set);

/* sigaddset/sigdelset/sigismember translate their signum argument like
 * sigaction()/kill() do, so either kernel bitmask constants (SIGTERM etc.)
 * or classic numbers are accepted. */
int sigaddset(sigset_t *set, int signum);
int sigdelset(sigset_t *set, int signum);
int sigismember(const sigset_t *set, int signum);

/* Number of kernel signal slots.  A signal's index is its bit number in the
 * mask (SIGKILL..SIGUSR2 above), and sigset_t is a 32-bit mask, so this stays
 * <= 32.  It must equal the kernel's NSIG (Cact/kernel/proc/task.h). */
#define KERNEL_NSIG 19

struct sigaction {
    sighandler_t sa_handler;
    sigset_t     sa_mask;
    int          sa_flags;
    void         (*sa_sigaction)(int, siginfo_t *, void *);
};

int          sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);
int          sigreturn(void);
int          sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
int          sigpending(sigset_t *set);
int          sigsuspend(const sigset_t *mask);
unsigned int alarm(unsigned int seconds);
int          setitimer(int which, const struct itimerval *new_value,
                       struct itimerval *old_value);

sighandler_t signal(int signum, sighandler_t handler);
int          kill(pid_t pid, int sig);
int          killpg(pid_t pgid, int sig);
int          raise(int sig);

/* The synchronous half of POSIX signals: wait for one of `set` and *consume* it
 * — no handler runs, `sig` comes back as the signal's number (the same value the
 * SIG* macros above hold, so `case SIGINT:` works).  POSIX has the caller block
 * the signals in `set` first.  All three return 0 or an error number directly
 * (they do not set errno); sigtimedwait() also reports EAGAIN on timeout, and a
 * NULL timeout waits forever.  SIGKILL/SIGSTOP cannot be waited for (they can be
 * neither blocked nor caught): they are ignored if present in `set`. */
int sigwait(const sigset_t *set, int *sig);
int sigwaitinfo(const sigset_t *set, siginfo_t *info);
int sigtimedwait(const sigset_t *set, siginfo_t *info, const struct timespec *timeout);

/* Like kill(), but the signal carries `value`, which the receiver reads back as
 * si_value (si_code SI_QUEUE).  A second sigqueue() for a signal that is already
 * pending does not queue another instance — POSIX's rule for non-realtime
 * signals, and the only behaviour this kernel has (a value is one word per
 * pending signal). */
int sigqueue(pid_t pid, int sig, const union sigval value);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* _SIGNAL_H */