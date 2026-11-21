#ifndef _SYS_IOCTL_H
#define _SYS_IOCTL_H

/* POSIX spelling: ioctl(2) is declared in ../unistd.h; the ioctl ABI
 * constants live in ../ioctl_abi.h. */
#include "unistd.h"
#include "ioctl_abi.h"

/* The _IO/_IOR/_IOW/_IOWR request encoders (Linux's layout).  The kernel's DRM
 * uapi headers define the same set, so the block is guarded: whichever header a
 * program reaches first defines them and the other defers.  CactOS's own ioctls
 * are plain 0x3xxx/0x4xxx numbers and do not use these. */
#ifndef _IOC
#define _IOC_NRBITS   8
#define _IOC_TYPEBITS 8
#define _IOC_SIZEBITS 14
#define _IOC_DIRBITS  2

#define _IOC_NRSHIFT   0
#define _IOC_TYPESHIFT (_IOC_NRSHIFT + _IOC_NRBITS)
#define _IOC_SIZESHIFT (_IOC_TYPESHIFT + _IOC_TYPEBITS)
#define _IOC_DIRSHIFT  (_IOC_SIZESHIFT + _IOC_SIZEBITS)

#define _IOC_NONE  0U
#define _IOC_WRITE 1U
#define _IOC_READ  2U

#define _IOC(dir, type, nr, size) \
    (((dir)  << _IOC_DIRSHIFT) | ((type) << _IOC_TYPESHIFT) | \
     ((nr)   << _IOC_NRSHIFT)  | ((size) << _IOC_SIZESHIFT))

#define _IO(type, nr)        _IOC(_IOC_NONE, (type), (nr), 0)
#define _IOR(type, nr, size) _IOC(_IOC_READ, (type), (nr), sizeof(size))
#define _IOW(type, nr, size) _IOC(_IOC_WRITE, (type), (nr), sizeof(size))
#define _IOWR(type, nr, size) _IOC(_IOC_READ | _IOC_WRITE, (type), (nr), sizeof(size))
#endif /* !_IOC */

#endif /* _SYS_IOCTL_H */
