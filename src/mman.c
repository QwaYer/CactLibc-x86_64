#include "mman.h"
#include "syscall.h"
#include <stdint.h>
#include "nodeio.h"
#include "ioctl_abi.h"
#include "fcntl.h"

/* mmap argument structure — must match kernel's mmap_args_t.  The three sizes
 * stay 32-bit: the mmap window (MMAP_BASE..MMAP_LIMIT) and the whole low user
 * half are below 4 GiB, so the kernel's own `do_mmap` takes 32-bit bounds and
 * the address is a hint in that window (see decision 9 in the kernel's
 * PORTING-x86_64.md). */
typedef struct {
    unsigned int addr;
    unsigned int length;
    int          prot;
    int          flags;
    int          fd;
    unsigned int offset;
} mmap_args_t;

void *mmap(void *addr, size_t length, int prot, int flags, int fd, unsigned int offset)
{
    mmap_args_t args;
    args.addr   = (unsigned int)(uintptr_t)addr;
    args.length = (unsigned int)length;
    args.prot   = prot;
    args.flags  = flags;
    args.fd     = fd;
    args.offset = offset;
    intptr_t ret = syscall(SYS_MMAP, (uintptr_t)&args, 0, 0);
    if (ret == -1)
        return MAP_FAILED;
    return (void *)(uintptr_t)ret;
}

int munmap(void *addr, size_t length)
{
    return (int)syscall(SYS_MUNMAP, (uintptr_t)addr, (uintptr_t)length, 0);
}

int mprotect(void *addr, size_t length, int prot)
{
    return (int)syscall(SYS_MPROTECT, (uintptr_t)addr, (uintptr_t)length, (uintptr_t)prot);
}

int msync(void *addr, size_t length, int flags)
{
    /* RAM-backed anonymous/memfd mappings need no writeback. */
    (void)addr; (void)length; (void)flags;
    return 0;
}

/* memfd_create(2): the /dev/memfd control node hands back a descriptor on a
 * fresh anonymous RAM-backed object. */
int memfd_create(const char *name, unsigned int flags)
{
    int fd = nio_open("/dev/memfd", O_RDWR);
    if (fd < 0) return -1;

    cact_memfd_create_arg_t a;
    a.name  = (char *)name;   /* optional */
    a.flags = (uint32_t)flags;
    int r = nio_map(nio_ioctl(fd, CACT_MEMFDCTL_CREATE, &a));
    nio_close(fd);
    return r;
}

/* madvise() is advisory by definition and CactOS's VM has no advice mechanism,
 * so the hint is accepted and dropped — see <mman.h>. */
int madvise(void *addr, size_t length, int advice) {
    (void)addr;
    (void)length;
    (void)advice;
    return 0;
}
