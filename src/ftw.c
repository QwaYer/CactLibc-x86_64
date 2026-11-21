/*
 * File-tree walk (nftw/ftw).
 *
 * CactOS's VFS gives a directory stream one entry at a time and has no readdir
 * d_type, so each entry is stat()ed to tell a directory from a file; the walk
 * is plain recursion over opendir/readdir.  A path is bounded by PATH_MAX (the
 * VFS stores a name in 128 bytes and a path in the same order of size), so the
 * recursion depth is bounded too — no explicit depth cap is needed.
 *
 * cctkfs carries no symlinks and no mount crossings inside one mount, so the
 * FTW_PHYS/FTW_CHDIR flags have nothing to change and FTW_MOUNT is honoured
 * with the start directory's st_dev (CactOS's st_dev is per-node).
 */

#include "ftw.h"
#include "dirent.h"
#include "string.h"
#include "errno.h"
#include "limits.h"

#ifndef PATH_MAX
#define PATH_MAX 256
#endif

/* ftw()'s callback has no FTW * argument, so it is adapted through a
 * file-scope pointer.  ftw() is documented as not required to be reentrant, and
 * this is the same trade every ftw() carries. */
static ftw_fn g_ftw_fn;

static int ftw_adapter(const char *path, const struct stat *sb, int type,
                       struct FTW *ftwbuf) {
    (void)ftwbuf;
    return g_ftw_fn(path, sb, type);
}

static int walk(const char *path, nftw_fn fn, int flags, int level, dev_t root_dev) {
    struct stat st;
    struct FTW f;
    const char *slash;

    f.level = level;
    slash = strrchr(path, '/');
    f.base  = slash ? (int)(slash - path + 1) : 0;

    if (stat(path, &st) != 0) {
        memset(&st, 0, sizeof(st));
        return fn(path, &st, FTW_NS, &f);
    }

    if (!S_ISDIR(st.st_mode))
        return fn(path, &st, FTW_F, &f);

    if (!(flags & FTW_DEPTH)) {
        int r = fn(path, &st, FTW_D, &f);
        if (r != 0) return r;
    }

    DIR *d = opendir(path);
    if (!d) {
        int r = fn(path, &st, FTW_DNR, &f);
        if (r != 0) return r;
    } else {
        struct dirent *de;
        while ((de = readdir(d)) != NULL) {
            char child[PATH_MAX];
            size_t dl, nl;
            struct stat cst;

            if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
                continue;

            dl = strlen(path);
            nl = strlen(de->d_name);
            if (dl + 1 + nl + 1 > sizeof(child))
                continue;                       /* cannot be named here */
            memcpy(child, path, dl);
            child[dl] = '/';
            memcpy(child + dl + 1, de->d_name, nl + 1);

            if ((flags & FTW_MOUNT) && stat(child, &cst) == 0 &&
                cst.st_dev != root_dev)
                continue;                       /* another filesystem */

            int r = walk(child, fn, flags, level + 1, root_dev);
            if (r != 0) { closedir(d); return r; }
        }
        closedir(d);
    }

    if (flags & FTW_DEPTH)
        return fn(path, &st, FTW_DP, &f);
    return 0;
}

int nftw(const char *path, nftw_fn fn, int fd_limit, int flags) {
    struct stat st;
    dev_t root_dev = 0;

    (void)fd_limit;   /* one directory stream is open at a time: never exceeded */
    if (path == NULL || fn == NULL) { errno = EINVAL; return -1; }

    if (stat(path, &st) == 0)
        root_dev = st.st_dev;

    return walk(path, fn, flags, 0, root_dev);
}

int ftw(const char *path, ftw_fn fn, int fd_limit) {
    if (path == NULL || fn == NULL) { errno = EINVAL; return -1; }
    g_ftw_fn = fn;
    return nftw(path, ftw_adapter, fd_limit, 0);
}
