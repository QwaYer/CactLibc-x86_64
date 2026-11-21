#ifndef FCNTL_H
#define FCNTL_H

/* C linkage for the C++ consumers (libc++, LLVM, Mesa) that include this
 * header; a mangled reference would match no CactLibc export. */
#ifdef __cplusplus
extern "C" {
#endif

#include "unistd.h"   /* off_t, pid_t (struct flock) */

/* Flags for open() */
#define O_RDONLY    0x0000
#define O_WRONLY    0x0001
#define O_RDWR      0x0002
#define O_ACCMODE   0x0003
#define O_CREAT     0x0040
#define O_TRUNC     0x0200
#define O_APPEND    0x0400
#define O_NONBLOCK  0x0800
#define O_CLOEXEC   0x80000
#define O_EXCL      0x0080

/* fcntl() commands */
#define F_DUPFD         0   /* duplicate file descriptor (>= arg) */
#define F_GETFD         1   /* get file descriptor flags */
#define F_SETFD         2   /* set file descriptor flags */
#define F_GETFL         3   /* get file status flags */
#define F_SETFL         4   /* set file status flags */
#define F_GETLK         5   /* advisory record lock: query */
#define F_SETLK         6   /* advisory record lock: set, non-blocking */
#define F_SETLKW        7   /* advisory record lock: set, blocking */
#define F_DUPFD_CLOEXEC 1030 /* duplicate file descriptor (>= arg) + FD_CLOEXEC */
#define F_ADD_SEALS     1033 /* memfd only: add the seal bits below */
#define F_GET_SEALS     1034 /* memfd only: the object's seal bits */

/* Advisory-lock types (struct flock.l_type).  CactOS backs these with its
 * process-local flock() table (see src/fcntl.c): locking is whole-file, so the
 * byte range in struct flock is not honoured, and the lock excludes only this
 * process. */
#define F_RDLCK  0
#define F_WRLCK  1
#define F_UNLCK  2

/* Linux's struct flock, x86-64 layout. */
struct flock {
    short l_type;
    short l_whence;
    off_t l_start;
    off_t l_len;
    pid_t l_pid;
};

/* memfd seals (F_ADD_SEALS / F_GET_SEALS).  The kernel enforces them:
 * F_SEAL_SHRINK / F_SEAL_GROW refuse the matching ftruncate, F_SEAL_WRITE
 * refuses writes, and F_SEAL_SEAL freezes the set.  Linux's values. */
#define F_SEAL_SEAL         0x0001
#define F_SEAL_SHRINK       0x0002
#define F_SEAL_GROW         0x0004
#define F_SEAL_WRITE        0x0008
#define F_SEAL_FUTURE_WRITE 0x0010

/* File descriptor flags (used with F_GETFD / F_SETFD) */
#define FD_CLOEXEC  1   /* close-on-exec */

int open(const char *pathname, int flags, ...);
int creat(const char *pathname, int mode);
int fcntl(int fd, int cmd, ...);

/* Reserve space for a file without writing it.  CactOS has no fallocate, so this
 * is POSIX's own fallback: grow the file with ftruncate() (which fills with
 * zeros), and do nothing if it is already that large.  Returns 0 or an error
 * number, as POSIX specifies. */
int posix_fallocate(int fd, off_t offset, off_t len);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
