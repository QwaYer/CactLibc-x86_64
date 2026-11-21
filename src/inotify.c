/*
 * inotify(7) — the ABI in name only.
 *
 * The Cact kernel has no inotify object, so every entry point reports ENOSYS.
 * A consumer that needs change notification (Mesa's shader-cache watcher) sees
 * the init fail and disables the watcher, which is the correct degradation;
 * adding a kernel inotify object is what turns this on.
 */

#include "inotify.h"
#include "errno.h"

int inotify_init(void) {
    return inotify_init1(0);
}

int inotify_init1(int flags) {
    (void)flags;
    errno = ENOSYS;
    return -1;
}

int inotify_add_watch(int fd, const char *pathname, uint32_t mask) {
    (void)fd;
    (void)pathname;
    (void)mask;
    errno = ENOSYS;
    return -1;
}

int inotify_rm_watch(int fd, int wd) {
    (void)fd;
    (void)wd;
    errno = ENOSYS;
    return -1;
}
