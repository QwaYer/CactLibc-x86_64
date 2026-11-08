#include "mman.h"
#include "syscall.h"
#include <stdint.h>

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
