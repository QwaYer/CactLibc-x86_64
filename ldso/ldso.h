#ifndef CACT_LDSO_H
#define CACT_LDSO_H

#include <stdint.h>
#include "tls_layout.h"

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int32_t  s32;

/* The ELF64 structures and the PT_/DT_/R_X86_64_ constants are shared with the
 * libc's dlopen() (both load the same x86_64 images), so they live in one header
 * rather than in two copies that can drift. */
#include "dl_elf.h"
#include "dl_objects.h"

/* auxv tags (Linux x86_64) */
#define AT_NULL    0
#define AT_PHDR    3
#define AT_PHENT   4
#define AT_PHNUM   5
#define AT_PAGESZ  6
#define AT_BASE    7
#define AT_ENTRY   9

/* raw syscall numbers — must match CactKernel syscalls.h */
#define SYS_OPEN   0
#define SYS_CLOSE  1
#define SYS_READ   2
#define SYS_WRITE  3
#define SYS_EXIT   8
#define SYS_MMAP   11
#define SYS_MUNMAP 12

#define PROT_READ   0x1
#define PROT_WRITE  0x2
#define PROT_EXEC   0x4

#define MAP_PRIVATE 0x02
#define MAP_FIXED   0x10
#define MAP_ANON    0x20
#define MAP_FAILED  ((void*)-1)

#define SO_PATH_MAX     256
#define LDSO_MAX_OBJS   8
#define LDSO_MAX_NEEDED 16
#define LDSO_SCRATCH    0x400000u   /* 4 MiB file scratch (cctkfs libs are << 1 MiB) */
/* Where the file scratch is mapped: below the kernel's mmap window
 * (MMAP_BASE = 0x80000000) and above every library's fixed address
 * (main 0x08000000, libc 0x10000000, ld.so 0x18000000), so it can never overlap
 * an image.  Mapped MAP_FIXED so the address is a property of ld.so rather than
 * of whatever the kernel's bump allocator happens to hand out. */
#define LDSO_SCRATCH_BASE 0x30000000u

long ldso_sc(unsigned long num, unsigned long a1, unsigned long a2, unsigned long a3);
void ldso_die(void);

#endif
