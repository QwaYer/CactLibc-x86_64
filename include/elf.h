#ifndef _ELF_H
#define _ELF_H

/* C linkage for the C++ consumers (libc++, LLVM, Mesa) that include this
 * header; a mangled reference would match no CactLibc export. */
#ifdef __cplusplus
extern "C" {
#endif

/*
 * <elf.h> — the ELF shapes and constants a *consumer* of mapped images needs,
 * as distinct from dl_elf.h, which is the two loaders' own working set.
 *
 * CactOS is x86_64-only and maps every image at its link-time address, so only
 * the 64-bit shapes exist here and no bias is ever applied to a value.  The
 * structs and constants the loaders already define (Elf64_Ehdr, Elf64_Phdr,
 * Elf64_Sym, Elf64_Dyn, Elf64_Rel/Rela, PT_*, DT_*, R_X86_64_*) come from
 * dl_elf.h — one definition, not two.
 */
#include <stdint.h>
#include "dl_elf.h"

/* The note header a PT_NOTE segment is made of.  Mesa's build_id.c walks the
 * program headers, selects PT_NOTE, and reinterprets the segment as these. */
typedef struct { uint32_t n_namesz; uint32_t n_descsz; uint32_t n_type; } Elf64_Nhdr;
typedef struct { uint32_t n_namesz; uint32_t n_descsz; uint32_t n_type; } Elf32_Nhdr;

#define PT_NOTE         4
#define NT_GNU_BUILD_ID 3

/* The section-header shape and a couple of its types, for completeness (the
 * CactOS loaders never read section headers — they drive from the dynamic
 * table — but a consumer reading an image off disk may). */
typedef struct {
    uint32_t sh_name;
    uint32_t sh_type;
    uint64_t sh_flags;
    uint64_t sh_addr;
    uint64_t sh_offset;
    uint64_t sh_size;
    uint32_t sh_link;
    uint32_t sh_info;
    uint64_t sh_addralign;
    uint64_t sh_entsize;
} Elf64_Shdr;

#define SHT_NULL     0
#define SHT_PROGBITS 1
#define SHT_SYMTAB   2
#define SHT_STRTAB   3
#define SHT_RELA     4
#define SHT_HASH     5
#define SHT_DYNAMIC  6
#define SHT_NOTE     7
#define SHT_NOBITS   8
#define SHT_REL      9
#define SHT_DYNSYM   11

/* glibc's 64-bit type spelling: ElfW(Nhdr) is Elf64_Nhdr here.  A program that
 * writes `#ifndef ElfW` falls back to this; Mesa's build_id.c does. */
#ifndef ElfW
#define ElfW(type) Elf64_##type
#endif

/* e_ident indexes and the class/data bytes this port ever sees. */
#define EI_NIDENT   16
#define EI_CLASS    4
#define EI_DATA     5
#define ELFCLASS64  2
#define ELFDATA2LSB 1

/* getauxval()/AT_* keys — u_cpu_detect.c names AT_HWCAP, and the rest are the
 * standard set so a program does not need its own copies. */
#define AT_NULL     0
#define AT_PHDR     3
#define AT_PHENT    4
#define AT_PHNUM    5
#define AT_PAGESZ   6
#define AT_BASE     7
#define AT_ENTRY    9
#define AT_UID      11
#define AT_EUID     12
#define AT_GID      13
#define AT_EGID     14
#define AT_PLATFORM 15
#define AT_HWCAP    16
#define AT_CLKTCK   17
#define AT_SECURE   23
#define AT_HWCAP2   26
#define AT_EXECFN   31

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* _ELF_H */
