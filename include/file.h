#ifndef _FILE_H
#define _FILE_H

/* C linkage for the C++ consumers (libc++, LLVM, Mesa) that include this
 * header; a mangled reference would match no CactLibc export. */
#ifdef __cplusplus
extern "C" {
#endif

/* flock() — advisory whole-file locking (POSIX <sys/file.h> spelling).
 *
 * CactOS has no kernel record-lock support yet, so this is a process-local
 * advisory lock — see src/flock.c for the exact limitation. */

#define LOCK_SH 1
#define LOCK_EX 2
#define LOCK_NB 4
#define LOCK_UN 8

int flock(int fd, int operation);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* _FILE_H */
