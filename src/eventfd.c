#include <eventfd.h>
#include "nodeio.h"
#include "ioctl_abi.h"
#include "errno.h"
#include "unistd.h"    /* read/write for the eventfd_t helpers */

int eventfd(unsigned int initval, int flags) {
    cact_eventfd_create_arg_t a;
    a.initval = initval;
    a.flags   = (uint32_t)flags & (CACT_EFD_SEMAPHORE | CACT_EFD_NONBLOCK | CACT_EFD_CLOEXEC);
    int r = nio_dev_cmd("eventfd", CACT_EVENTFDCTL_CREATE, &a);
    if (r < 0) { errno = -r; return -1; }
    return r;
}

/* The counter is one 8-byte word, so the helpers are a read or a write of it;
 * a short transfer is an I/O error. */
int eventfd_read(int fd, eventfd_t *value) {
    if (!value) { errno = EINVAL; return -1; }
    ssize_t n = read(fd, value, sizeof(*value));
    if (n != (ssize_t)sizeof(*value)) {
        if (n >= 0) errno = EIO;
        return -1;
    }
    return 0;
}

int eventfd_write(int fd, eventfd_t value) {
    ssize_t n = write(fd, &value, sizeof(value));
    if (n != (ssize_t)sizeof(value)) {
        if (n >= 0) errno = EIO;
        return -1;
    }
    return 0;
}
