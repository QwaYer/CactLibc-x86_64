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
#define LDSO_MAX_OBJS   32
#define LDSO_MAX_NEEDED 48
#define LDSO_SCRATCH    0x400000u   /* scratch per load — see the note below */

/* ── image address map ──────────────────────────────────────────────────
 * The user half of the address space is [0x08000000, 0xC0000000) and CactOS
 * maps every image at its link-time address (bias 0), so each shared library
 * needs its own disjoint base.  That base lives in the library's linker script,
 * not here, but this table is the single place the map is written down: a .so
 * whose base is not listed must be added here before it ships.
 *
 *   fixed, already present
 *     0x08000000  the main executable
 *     0x10000000  clibc.so                       (libc.ld)
 *     0x18000000  ld.so                          (ldso/ldso.ld)
 *     0x30000000  ld.so file scratch (4 MiB)     (LDSO_SCRATCH_BASE)
 *     0x80000000  the kernel's mmap window       (MMAP_BASE)
 *
 *   library slab — 16 MiB slots, from 0x20000000
 *     0x20000000  libz.so
 *     0x21000000  libffi.so
 *     0x22000000  libexpat.so
 *     0x23000000  libdrm.so
 *     0x24000000  libwayland-client.so
 *     0x25000000  libwayland-server.so
 *     0x26000000  libwayland-egl.so
 *     0x27000000  libwayland-cursor.so
 *     0x28000000  libc++.so
 *     0x29000000  libc++abi.so
 *     0x2A000000 … 0x2F000000  unassigned (Mesa frontends, pixman, libxkbcommon)
 *
 *   large images — explicit size, above the scratch
 *     0x40000000  libLLVM.so    (reserved to 0x60000000)
 *     0x60000000  libgallium.so (reserved to 0x70000000)
 *
 * The scratch is mapped MAP_FIXED so its address is a property of ld.so rather
 * than of whatever the kernel's bump allocator hands out; it sits below the mmap
 * window and above every library base, so it can never overlap an image.
 *
 * NOTE: load_library() reads a whole .so through this scratch before mapping it,
 * so nothing larger than LDSO_SCRATCH loads yet.  Every library in the slab above
 * fits; the two large images need the read converted to a forward stream first. */
#define LDSO_SCRATCH_BASE 0x30000000u

long ldso_sc(unsigned long num, unsigned long a1, unsigned long a2, unsigned long a3);
void ldso_die(void);

#endif
