#ifndef _STATFS_H
#define _STATFS_H

/* C linkage for the C++ consumers (libc++, LLVM, Mesa) that include this
 * header; a mangled reference would match no CactLibc export. */
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include "unistd.h"   /* fsblkcnt_t, fsfilcnt_t */

/* The POSIX/BSD `struct statfs`, as a caller (LLVM's disk_space()) expects it.
 * CactOS's own filesystem statistics live in the smaller `cact_statfs_t` (the
 * VFS writes that one); statfs()/fstatfs() read it and present this.  The
 * layout follows glibc's: 64-bit counters, and the fields glibc has that the
 * VFS does not fill (fsid, flags) are zero. */

struct statfs {
    long        f_type;      /* filesystem type (hash of the fstype name) */
    long        f_bsize;     /* optimal transfer block size */
    fsblkcnt_t  f_blocks;    /* total data blocks in the filesystem */
    fsblkcnt_t  f_bfree;     /* free blocks */
    fsblkcnt_t  f_bavail;    /* free blocks available to unprivileged users */
    fsfilcnt_t  f_files;     /* total inodes */
    fsfilcnt_t  f_ffree;     /* free inodes */
    struct { int __val[2]; } f_fsid;   /* filesystem id — not tracked; zero */
    long        f_namelen;   /* maximum filename length */
    long        f_frsize;    /* fragment size (= f_bsize) */
    long        f_flags;     /* mount flags — not tracked; zero */
    long        f_spare[4];
};

int statfs(const char *path, struct statfs *buf);
int fstatfs(int fd, struct statfs *buf);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* _STATFS_H */
