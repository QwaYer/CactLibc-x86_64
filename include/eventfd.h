#ifndef _SYS_EVENTFD_H
#define _SYS_EVENTFD_H

/* C linkage for the C++ consumers (libc++, LLVM, Mesa) that include this
 * header; a mangled reference would match no CactLibc export. */
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <fcntl.h>

/* Linux eventfd(2) API as served by CactOS via /dev/eventfd + ioctls. */

#define EFD_SEMAPHORE 0x0001

#ifndef EFD_CLOEXEC
#define EFD_CLOEXEC O_CLOEXEC
#endif
#ifndef EFD_NONBLOCK
#define EFD_NONBLOCK O_NONBLOCK
#endif

int eventfd(unsigned int initval, int flags);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* _SYS_EVENTFD_H */
