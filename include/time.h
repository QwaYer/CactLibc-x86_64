#ifndef _TIME_H
#define _TIME_H

/* C linkage for the C++ consumers (libc++, LLVM, Mesa) that include this
 * header; a mangled reference would match no CactLibc export. */
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

typedef long time_t;

/* Clock and timer identifiers.  CactOS has CLOCK_REALTIME and CLOCK_MONOTONIC
 * (see clock_gettime) and no POSIX per-process timers, so timer_t names nothing
 * the kernel can start; both types exist so a consumer's declarations compile. */
typedef int clockid_t;
typedef int timer_t;

/* clock()/CLOCKS_PER_SEC.  CactOS exposes no per-process CPU accounting to
 * userspace, so clock() reports elapsed CLOCK_MONOTONIC time; the constant is
 * the POSIX-mandated 1000000 (1 µs ticks) even though the kernel timer is 1 ms. */
typedef long clock_t;
#define CLOCKS_PER_SEC 1000000

struct tm {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
};

struct timeval {
    long tv_sec;
    long tv_usec;
};

struct timezone {
    int tz_minuteswest;
    int tz_dsttime;
};

struct timespec {
    long tv_sec;
    long tv_nsec;
};

struct itimerspec {
    struct timespec it_interval;   /* period between expirations         */
    struct timespec it_value;      /* time until first expiration        */
};

#define CLOCK_REALTIME  0
#define CLOCK_MONOTONIC 1
#define TIMER_ABSTIME   1

time_t time(time_t *t);
double difftime(time_t time1, time_t time0);

/* Timezone: `TZ` is a POSIX TZ string (`std offset [dst [offset][,start,end]]`).
 * An empty or zone-file name (one containing '/') has no database to read here
 * and falls back to UTC.  tzset() is called implicitly by the functions below. */
void tzset(void);
extern char *tzname[2];
extern long timezone;   /* seconds west of UTC, standard time */
extern int  daylight;   /* nonzero when DST is ever used */

struct tm *gmtime(const time_t *timep);
struct tm *gmtime_r(const time_t *timep, struct tm *result);
struct tm *localtime(const time_t *timep);
struct tm *localtime_r(const time_t *timep, struct tm *result);
time_t mktime(struct tm *tm);
char *asctime(const struct tm *tm);
char *asctime_r(const struct tm *tm, char *buf);
char *ctime(const time_t *timep);
char *ctime_r(const time_t *timep, char *buf);
int gettimeofday(struct timeval *tv, void *tz);
int clock_gettime(int clkid, struct timespec *tp);
/* Elapsed processor time since an arbitrary point (see CLOCKS_PER_SEC). */
clock_t clock(void);
/* Sets the wall clock (CLOCK_REALTIME only) through the kernel's
 * /dev/sys CACT_SYSCTL_SETWALLCLOCK; needs root. */
int clock_settime(int clkid, const struct timespec *tp);
/* clock_getres reports the kernel's timer resolution (1 ms). */
int clock_getres(int clkid, struct timespec *res);
int nanosleep(const struct timespec *req, struct timespec *rem);
/* The clock-taking sleep: like nanosleep(), but on the named clock, optionally
 * to an absolute deadline (TIMER_ABSTIME), and it returns the error number
 * itself (POSIX) rather than -1/errno. */
int clock_nanosleep(int clkid, int flags, const struct timespec *req,
                    struct timespec *rem);

/* C locale, and the zone is UTC (localtime() == gmtime()). */
size_t strftime(char *s, size_t max, const char *format, const struct tm *tm);
char  *strptime(const char *s, const char *format, struct tm *tm);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
