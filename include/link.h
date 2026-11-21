#ifndef _LINK_H
#define _LINK_H

/* C linkage for the C++ consumers (libc++, LLVM, Mesa) that include this
 * header; a mangled reference would match no CactLibc export. */
#ifdef __cplusplus
extern "C" {
#endif

/* dl_iterate_phdr(): walk the objects that are mapped into this process.
 *
 * CactOS maps every image at its link-time address and never applies a bias, so
 * `dlpi_addr` is always 0 and the program headers the callback receives carry
 * absolute addresses.  The ELF header of one of these images is *not* mapped
 * (the first PT_LOAD starts at file offset 0x1000), so the loaders keep a copy
 * of each object's program headers and hand that to the callback.
 *
 * The four glibc extension fields are present for size compatibility and are
 * zero: there is no per-object TLS module id here, and nothing is ever
 * unloaded from the initial image. */

#include <stddef.h>
#include "dl_elf.h"
/* As glibc's <link.h> does: a consumer that reaches the program headers through
 * dl_iterate_phdr() also gets the ELF types and ElfW(), which is how Mesa's
 * build_id.c names ElfW(Nhdr)/PT_NOTE. */
#include "elf.h"

struct dl_phdr_info {
    unsigned long     dlpi_addr;      /* load bias — always 0 here */
    const char       *dlpi_name;      /* "" for the program, else the soname */
    const Elf64_Phdr *dlpi_phdr;
    uint16_t          dlpi_phnum;
    /* glibc extensions */
    unsigned long long dlpi_adds;
    unsigned long long dlpi_subs;
    size_t             dlpi_tls_modid;
    void              *dlpi_tls_data;
};

/* The callback returns 0 to continue the walk, nonzero to stop it. */
int dl_iterate_phdr(int (*callback)(struct dl_phdr_info *info, size_t size, void *data),
                    void *data);

/* The public prefix of a link map, exactly as glibc declares it: what
 * dlinfo(RTLD_DI_LINKMAP) hands out.  CactOS maps every image at its link-time
 * address, so `l_addr` is always 0; `l_name` is whatever the loaders know the
 * object by (the soname, the path it was opened with, or "<main>" for the
 * program, where glibc has an empty name); `l_next`/`l_prev` chain the objects
 * in load order, the program first. */
struct link_map {
    unsigned long    l_addr;    /* load bias — always 0 here */
    char            *l_name;
    Elf64_Dyn       *l_ld;      /* the object's PT_DYNAMIC, or 0 for none */
    struct link_map *l_next, *l_prev;
};

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
