#include "stat.h"
#include "statfs.h"
#include "nodeio.h"
#include "fcntl.h"
#include "errno.h"
#include "stat.h"
#include "utime.h"
#include <stdint.h>

/* fstatat(2).  CactOS's VFS cannot name the node behind a descriptor (see the
 * fchmod note above), so a name *relative to a directory descriptor* is not
 * resolvable; AT_FDCWD and an absolute name are, and AT_EMPTY_PATH stats the
 * descriptor itself. */
int fstatat(int dirfd, const char *pathname, struct stat *buf, int flags) {
    if (!pathname || !buf) { errno = EINVAL; return -1; }

    if (pathname[0] == '\0') {
        if (flags & AT_EMPTY_PATH) return fstat(dirfd, buf);
        errno = ENOENT;
        return -1;
    }
    if (dirfd != AT_FDCWD && pathname[0] != '/') {
        errno = ENOSYS;   /* no fd -> path mapping */
        return -1;
    }
    if (flags & AT_SYMLINK_NOFOLLOW)
        return lstat(pathname, buf);   /* lstat goes through statx, which honours the flag */
    return stat(pathname, buf);
}

/* CactOS's VFS cannot set a file's times — the kernel exposes no ioctl for them
 * and there is no futimens() either (see <utime.h>).  Report the gap honestly
 * instead of returning success. */
int utime(const char *path, const struct utimbuf *times) {
    (void)times;
    if (!path) { errno = ENOENT; return -1; }
    errno = ENOSYS;
    return -1;
}

/* stat()/fstat() go through the kernel's small `cact_stat_t`, which carries only
 * ino/mode/size/type/rdev (the leading 24 bytes of struct stat).  A ported
 * program (LLVM) also reads the POSIX members, so fill the ones CactOS can
 * report honestly: one filesystem (dev 0), a single link, root ownership, and
 * epoch-zero times.  lstat() goes through the richer statx and fills these for
 * real. */
static void fill_derived(struct stat *buf) {
    buf->st_dev   = 0;
    buf->st_nlink = 1;
    buf->st_uid   = 0;
    buf->st_gid   = 0;
    buf->st_atime = 0;
    buf->st_mtime = 0;
    /* The VFS does not report a block size, so name the one thing it does use:
     * the page (4 KiB), which is also what a caller sizing a read buffer wants.
     * st_blocks follows from the byte length at 512 bytes per block. */
    buf->st_blksize = 4096;
    buf->st_blocks  = (blkcnt_t)((buf->st_size + 511) / 512);
}

int stat(const char *path, struct stat *buf) {
    if (!path) { errno = ENOENT; return -1; }

    /* "/" directly */
    if (path[0] == '/' && path[1] == '\0') {
        int fd = nio_open("/", O_RDONLY);
        if (fd < 0) return -1;
        int r = nio_ioctl(fd, CACT_FDCTL_FSTAT, buf);
        nio_close(fd);
        if (r < 0) return nio_map(r);
        fill_derived(buf);
        return 0;
    }

    /* strip trailing '/' (except the root) */
    char pbuf[512];
    size_t i = 0;
    while (path[i] && i < sizeof(pbuf) - 1) { pbuf[i] = path[i]; i++; }
    pbuf[i] = '\0';
    while (i > 1 && pbuf[i - 1] == '/') pbuf[--i] = '\0';

    char base[128];
    int  fd = nio_open_parent(pbuf, base, sizeof(base));
    if (fd < 0) return -1;
    cact_statat_arg_t a;
    a.name = base;
    a.buf  = (cact_stat_t *)buf;   /* the leading 24 bytes are cact_stat_t */
    int r = nio_ioctl(fd, CACT_DIRCTL_STAT, &a);
    nio_close(fd);
    if (r < 0) return nio_map(r);
    fill_derived(buf);
    return 0;
}

int fstat(int fd, struct stat *buf) {
    int r = nio_ioctl(fd, CACT_FDCTL_FSTAT, buf);
    if (r < 0) return nio_map(r);
    fill_derived(buf);
    return 0;
}

int chmod(const char *path, int mode) {
    char base[128];
    int  fd = nio_open_parent(path, base, sizeof(base));
    if (fd < 0) return -1;
    cact_chmod_arg_t a;
    a.name = base;
    a.mode = (uint32_t)mode;
    int r = nio_ioctl(fd, CACT_DIRCTL_CHMOD, &a);
    nio_close(fd);
    return nio_map(r);
}

/* POSIX fchmod().  CactOS changes a mode through the file's parent directory
 * (CACT_DIRCTL_CHMOD) and the VFS offers no fd -> path mapping, so there is no
 * way to name the node a descriptor refers to.  A fd-level chmod would be a new
 * kernel ioctl, mirroring the fd-level stat (CACT_FDCTL_FSTAT); until it
 * exists this fails rather than pretend. */
int fchmod(int fd, mode_t mode) {
    (void)fd;
    (void)mode;
    errno = ENOSYS;
    return -1;
}

/* POSIX fchown(), with the same restriction as fchmod(): ownership is changed
 * through the parent directory (CACT_DIRCTL_CHOWN) and the VFS has no fd ->
 * path mapping, so a descriptor cannot name its node.  A fd-level chown would
 * be a new kernel ioctl, mirroring the fd-level stat (CACT_FDCTL_FSTAT); until
 * it exists this fails rather than pretend. */
int fchown(int fd, uid_t uid, gid_t gid) {
    (void)fd;
    (void)uid;
    (void)gid;
    errno = ENOSYS;
    return -1;
}

int chown(const char *path, int uid, int gid) {
    char base[128];
    int  fd = nio_open_parent(path, base, sizeof(base));
    if (fd < 0) return -1;
    cact_chown_arg_t a;
    a.name = base;
    a.uid  = (uint32_t)uid;
    a.gid  = (uint32_t)gid;
    int r = nio_ioctl(fd, CACT_DIRCTL_CHOWN, &a);
    nio_close(fd);
    return nio_map(r);
}

int lstat(const char *path, struct stat *buf) {
    if (!path || !buf) { errno = ENOENT; return -1; }

    /* "/" is never a symlink: nothing to not-follow. */
    if (path[0] == '/' && path[1] == '\0') return stat(path, buf);

    char pbuf[512];
    size_t i = 0;
    while (path[i] && i < sizeof(pbuf) - 1) { pbuf[i] = path[i]; i++; }
    pbuf[i] = '\0';
    while (i > 1 && pbuf[i - 1] == '/') pbuf[--i] = '\0';

    char base[128];
    int  fd = nio_open_parent(pbuf, base, sizeof(base));
    if (fd < 0) return -1;

    /* The 4-word cact_stat cannot carry a link target, so ask the rich stat
     * with NOFOLLOW and keep the fields struct stat has. */
    cact_statx_t sx;
    cact_statx_arg_t a;
    a.name  = base;
    a.buf   = &sx;
    a.flags = CACT_AT_SYMLINK_NOFOLLOW;
    int r = nio_ioctl(fd, CACT_DIRCTL_STATX, &a);
    nio_close(fd);
    if (r < 0) return nio_map(r);

    buf->st_ino  = sx.ino;
    buf->st_mode = sx.mode;
    buf->st_size = sx.size;
    buf->st_type = sx.type;
    buf->st_rdev = sx.rdev;
    /* The statx path carries the POSIX members the small stat cannot. */
    buf->st_dev   = 0;
    buf->st_nlink = sx.nlink;
    buf->st_uid   = sx.uid;
    buf->st_gid   = sx.gid;
    buf->st_atime = (int64_t)sx.atime;
    buf->st_mtime = (int64_t)sx.mtime;
    buf->st_blksize = 4096;
    buf->st_blocks  = (blkcnt_t)((sx.size + 511) / 512);
    return 0;
}

/* The kernel reports the small `cact_statfs_t`; a caller gets the POSIX
 * `struct statfs`.  The VFS does not carry an fsid or mount flags, so those
 * stay zero, and f_frsize follows f_bsize. */
static void statfs_from_cact(const cact_statfs_t *c, struct statfs *buf) {
    buf->f_type    = (long)c->f_type;
    buf->f_bsize   = (long)c->f_bsize;
    buf->f_blocks  = (fsblkcnt_t)c->f_blocks;
    buf->f_bfree   = (fsblkcnt_t)c->f_bfree;
    buf->f_bavail  = (fsblkcnt_t)c->f_bavail;
    buf->f_files   = (fsfilcnt_t)c->f_files;
    buf->f_ffree   = (fsfilcnt_t)c->f_ffree;
    buf->f_fsid.__val[0] = 0;
    buf->f_fsid.__val[1] = 0;
    buf->f_namelen = (long)c->f_namelen;
    buf->f_frsize  = (long)c->f_bsize;
    buf->f_flags   = 0;
    for (int i = 0; i < 4; i++) buf->f_spare[i] = 0;
}

/* Filesystem statistics for the filesystem containing `path`. */
int statfs(const char *path, struct statfs *buf) {
    if (!path || !buf) { errno = ENOENT; return -1; }

    if (path[0] == '/' && path[1] == '\0') {
        int fd = nio_open("/", O_RDONLY);
        if (fd < 0) return -1;
        cact_statfs_t c;
        cact_statfs_arg_t a;
        a.name = 0;              /* the directory's own filesystem */
        a.buf  = &c;
        int r = nio_ioctl(fd, CACT_DIRCTL_STATFS, &a);
        nio_close(fd);
        if (r < 0) return nio_map(r);
        statfs_from_cact(&c, buf);
        return 0;
    }

    char pbuf[512];
    size_t i = 0;
    while (path[i] && i < sizeof(pbuf) - 1) { pbuf[i] = path[i]; i++; }
    pbuf[i] = '\0';
    while (i > 1 && pbuf[i - 1] == '/') pbuf[--i] = '\0';

    char base[128];
    int  fd = nio_open_parent(pbuf, base, sizeof(base));
    if (fd < 0) return -1;
    cact_statfs_t c;
    cact_statfs_arg_t a;
    a.name = base;
    a.buf  = &c;
    int r = nio_ioctl(fd, CACT_DIRCTL_STATFS, &a);
    nio_close(fd);
    if (r < 0) return nio_map(r);
    statfs_from_cact(&c, buf);
    return 0;
}

/* The fd form: the directory's own filesystem, the same way statfs("/") asks. */
int fstatfs(int fd, struct statfs *buf) {
    if (!buf) { errno = ENOENT; return -1; }
    cact_statfs_t c;
    cact_statfs_arg_t a;
    a.name = 0;
    a.buf  = &c;
    int r = nio_ioctl(fd, CACT_DIRCTL_STATFS, &a);
    if (r < 0) return nio_map(r);
    statfs_from_cact(&c, buf);
    return 0;
}

/* Rich stat (statx).  Follows a final symlink unless CACT_AT_SYMLINK_NOFOLLOW. */
int cact_statx(const char *path, cact_statx_t *buf, uint32_t flags) {
    if (!path || !buf) { errno = ENOENT; return -1; }

    if (path[0] == '/' && path[1] == '\0') {
        int fd = nio_open("/", O_RDONLY);
        if (fd < 0) return -1;
        int r = nio_ioctl(fd, CACT_FDCTL_FSTATX, buf);
        nio_close(fd);
        return nio_map(r);
    }

    char pbuf[512];
    size_t i = 0;
    while (path[i] && i < sizeof(pbuf) - 1) { pbuf[i] = path[i]; i++; }
    pbuf[i] = '\0';
    while (i > 1 && pbuf[i - 1] == '/') pbuf[--i] = '\0';

    char base[128];
    int  fd = nio_open_parent(pbuf, base, sizeof(base));
    if (fd < 0) return -1;
    cact_statx_arg_t a;
    a.name  = base;
    a.buf   = buf;
    a.flags = flags;
    int r = nio_ioctl(fd, CACT_DIRCTL_STATX, &a);
    nio_close(fd);
    return nio_map(r);
}
