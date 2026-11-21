#include "unistd.h"
#include "errno.h"
#include "nodeio.h"
#include "string.h"

/* The kernel's user page size and per-process descriptor table. */
#define CACT_PAGE_SIZE 4096
#define CACT_MAX_FD    256      /* Cact/kernel/proc/task.h: MAX_FD */

int getpagesize(void) {
    return CACT_PAGE_SIZE;
}

/* Count the "processor" lines of /proc/cpuinfo — the only per-CPU view the
 * kernel offers.  One is the floor: an unreadable or empty file still means
 * "at least the CPU we are running on". */
static int cpu_count(void) {
    static char buf[8192];
    int n = nio_read_file("/proc/cpuinfo", buf, sizeof(buf) - 1);
    if (n <= 0) return 1;
    buf[n] = '\0';

    int count = 0;
    for (const char *p = buf; (p = strstr(p, "processor")) != 0; p++) count++;
    return count > 0 ? count : 1;
}

/* A "<label>: <n> kB" line of /proc/meminfo, converted to pages.  -1 means
 * "unknown", which is what sysconf's callers treat as an unsupported limit. */
static long mem_pages(const char *label) {
    static char buf[4096];
    int n = nio_read_file("/proc/meminfo", buf, sizeof(buf) - 1);
    if (n <= 0) return -1;
    buf[n] = '\0';

    const char *p = strstr(buf, label);
    if (!p) return -1;
    p += strlen(label);
    while (*p == ' ' || *p == '\t') p++;
    if (*p < '0' || *p > '9') return -1;

    long kb = 0;
    while (*p >= '0' && *p <= '9') kb = kb * 10 + (*p++ - '0');
    return kb / (CACT_PAGE_SIZE / 1024);
}

long sysconf(int name) {
    switch (name) {
    case _SC_PAGESIZE:         return CACT_PAGE_SIZE;
    case _SC_CLK_TCK:          return 100;          /* nominal: no tick API is exposed */
    case _SC_OPEN_MAX:         return CACT_MAX_FD;
    case _SC_CHILD_MAX:        return 1024;
    case _SC_ARG_MAX:          return 131072;
    case _SC_VERSION:          return 200809L;      /* POSIX.1-2008 */
    /* "No fixed limit", as glibc answers; a caller (LLVM) then falls back to
     * its own default buffer size for getpw*_r(). */
    case _SC_GETPW_R_SIZE_MAX: return -1;
    case _SC_NPROCESSORS_CONF:
    case _SC_NPROCESSORS_ONLN: return cpu_count();
    case _SC_PHYS_PAGES:       return mem_pages("MemTotal:");
    case _SC_AVPHYS_PAGES:     return mem_pages("MemFree:");
    default:
        errno = EINVAL;
        return -1;
    }
}
