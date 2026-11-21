#ifndef _UTIME_H
#define _UTIME_H

/* C linkage for the C++ consumers (libc++, LLVM, Mesa) that include this
 * header; a mangled reference would match no CactLibc export. */
#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>

struct utimbuf {
    time_t actime;    /* access time      */
    time_t modtime;   /* modification time */
};

/* CactOS's VFS cannot set a file's times — the kernel exposes no ioctl for
 * them and there is no futimens() either (the LLVM port records the same gap
 * for futimes/futimens).  utime() therefore reports ENOSYS rather than
 * pretending the times changed. */
int utime(const char *path, const struct utimbuf *times);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* _UTIME_H */
