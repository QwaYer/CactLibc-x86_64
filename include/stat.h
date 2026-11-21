#ifndef _CACT_STAT_H
#define _CACT_STAT_H

/* C linkage for the C++ consumers (libc++, LLVM, Mesa) that include this
 * header; a mangled reference would match no CactLibc export. */
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <unistd.h>   /* dev_t, ino_t, nlink_t, uid_t, gid_t, off_t */

#define S_IFMT   0xF000
#define S_IFREG  0x8000
#define S_IFDIR  0x4000
#define S_IFCHR  0x2000
#define S_IFBLK  0x6000
#define S_IFIFO  0x1000
#define S_IFLNK  0xA000
#define S_IFSOCK 0xC000

#define S_ISREG(m)  (((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m)  (((m) & S_IFMT) == S_IFDIR)
#define S_ISCHR(m)  (((m) & S_IFMT) == S_IFCHR)
#define S_ISBLK(m)  (((m) & S_IFMT) == S_IFBLK)
#define S_ISFIFO(m) (((m) & S_IFMT) == S_IFIFO)
#define S_ISLNK(m)  (((m) & S_IFMT) == S_IFLNK)
#define S_ISSOCK(m) (((m) & S_IFMT) == S_IFSOCK)

#define S_ISUID  04000
#define S_ISGID  02000
#define S_ISVTX  01000

#define S_IRWXU  00700
#define S_IRUSR  00400
#define S_IWUSR  00200
#define S_IXUSR  00100

#define S_IRWXG  00070
#define S_IRGRP  00040
#define S_IWGRP  00020
#define S_IXGRP  00010

#define S_IRWXO  00007
#define S_IROTH  00004
#define S_IWOTH  00002
#define S_IXOTH  00001

/* The first five members are layout-identical to the kernel's `cact_stat_t`
 * (see ioctl_abi.h): the VFS writes that struct straight into the buffer, 24
 * bytes of it.  The members after them are the POSIX ones a ported program
 * reads (LLVM's file layer wants st_dev/st_nlink/st_uid/st_gid and the
 * timestamps); the VFS's small stat does not carry them, so stat()/fstat() fill
 * what CactOS can honestly report and lstat() (which goes through the richer
 * statx) fills the rest.  st_type is a CactOS extension kept for its own
 * utilities. */
struct stat {
    uint32_t st_ino;
    uint32_t st_mode;
    uint64_t st_size;
    uint32_t st_type;
    /* 0 unless the node is a device that reports a number; the DRM nodes do
     * (see the rdev comment in ioctl_abi.h). */
    uint32_t st_rdev;

    dev_t   st_dev;
    nlink_t st_nlink;
    uid_t   st_uid;
    gid_t   st_gid;
    int64_t st_atime;   /* seconds since the Unix epoch */
    int64_t st_mtime;
    blksize_t st_blksize;   /* preferred I/O size; the VFS's page size */
    blkcnt_t  st_blocks;    /* 512-byte blocks the file occupies */
};

int stat(const char *path, struct stat *buf);
int lstat(const char *path, struct stat *buf);
int fstat(int fd, struct stat *buf);
int chmod(const char *path, int mode);
/* POSIX fchmod().  CactOS's VFS has no fd -> path mapping, so there is no way
 * to name the node behind a descriptor and this fails with ENOSYS; see the
 * note in src/stat.c. */
int fchmod(int fd, mode_t mode);
int chown(const char *path, int uid, int gid);

/* fstatat(2).  CactOS's VFS has no descriptor-to-path mapping, so a name
 * *relative to a directory descriptor* cannot be resolved — AT_FDCWD or an
 * absolute name works.  The AT_* values are Linux's, and AT_SYMLINK_NOFOLLOW
 * carries the same bit the kernel's statx honours (CACT_AT_SYMLINK_NOFOLLOW). */
#define AT_FDCWD            (-100)
#define AT_SYMLINK_NOFOLLOW 0x100
#define AT_SYMLINK_FOLLOW   0x400
#define AT_EMPTY_PATH       0x1000
int fstatat(int dirfd, const char *pathname, struct stat *buf, int flags);

#include <ioctl_abi.h>

// Rich stat; `flags` may set CACT_AT_SYMLINK_NOFOLLOW.
int cact_statx(const char *path, cact_statx_t *buf, uint32_t flags);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif