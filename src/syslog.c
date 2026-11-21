/*
 * syslog(3).
 *
 * CactOS programs reach the log daemon through their own stderr/the console,
 * not through a /dev/log socket in the libc, so this writes the formatted line
 * to stderr prefixed with the ident — the closest thing to glibc's LOG_PERROR.
 * The priority mask (setlogmask) is honoured; the facility is recorded but
 * unused, since there is no socket to tag.
 */

#include "syslog.h"
#include "stdio.h"
#include <stdarg.h>

static const char *g_ident    = "cact";
static int         g_facility = LOG_USER;
static int         g_mask     = 0xff;   /* all priorities enabled by default */

void openlog(const char *ident, int option, int facility) {
    (void)option;
    if (ident) g_ident = ident;
    g_facility = facility;
}

void closelog(void) { }

int setlogmask(int mask) {
    int old = g_mask;
    if (mask != 0) g_mask = mask;
    return old;
}

void syslog(int priority, const char *format, ...) {
    if (!(g_mask & LOG_MASK(priority & 7))) return;
    (void)g_facility;

    va_list ap;
    fprintf(stderr, "%s: ", g_ident);
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);
    fputc('\n', stderr);
}
