#ifndef _FTW_H
#define _FTW_H

/* C linkage for the C++ consumers (libc++, LLVM, Mesa) that include this
 * header; a mangled reference would match no CactLibc export. */
#ifdef __cplusplus
extern "C" {
#endif

#include <sys/stat.h>     /* struct stat — the callback's second argument */

/*
 * File-tree walk (nftw/ftw).
 *
 * The type codes and the flag bits are the standard ones; the walk itself is
 * in src/ftw.c.  CactOS's cctkfs carries no symlinks, so the FTW_SL/FTW_SLN
 * codes and FTW_PHYS/FTW_CHDIR only exist for source compatibility — nothing
 * here ever produces them.
 */
#define FTW_F    0        /* a regular file */
#define FTW_D    1        /* a directory, pre-order */
#define FTW_DNR  2        /* a directory that could not be read */
#define FTW_NS   3        /* stat() failed */
#define FTW_SL   4        /* a symbolic link (never on CactOS) */
#define FTW_DP   5        /* a directory, post-order (FTW_DEPTH) */
#define FTW_SLN  6        /* a dangling symbolic link (never on CactOS) */

#define FTW_PHYS  0x01    /* do not follow symlinks */
#define FTW_MOUNT 0x02    /* stay within one filesystem */
#define FTW_DEPTH 0x04    /* report directories after their contents */
#define FTW_CHDIR 0x08    /* chdir() into each directory (not honoured) */

/* The fourth argument of an nftw() callback: `base` is the offset of the
 * entry's own name within `fpath`, `level` the depth (the start path is 0). */
struct FTW {
    int base;
    int level;
};

typedef int (*nftw_fn)(const char *fpath, const struct stat *sb, int typeflag,
                       struct FTW *ftwbuf);
typedef int (*ftw_fn)(const char *fpath, const struct stat *sb, int typeflag);

int nftw(const char *path, nftw_fn fn, int fd_limit, int flags);
int ftw(const char *path, ftw_fn fn, int fd_limit);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* _FTW_H */
