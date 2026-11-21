#ifndef CACT_DL_ELF_H
#define CACT_DL_ELF_H

/* C linkage for the C++ consumers (libc++, LLVM, Mesa) that include this
 * header; a mangled reference would match no CactLibc export. */
#ifdef __cplusplus
extern "C" {
#endif

/* ELF64 shapes and constants shared by the two loaders: ld.so (which maps the
 * program and its DT_NEEDED libraries) and the libc's dlopen() (which maps
 * whatever a program asks for later).  Both work on the same x86_64 images, and
 * CactOS maps every image at its link-time address, so no bias is ever applied
 * to a symbol value or a relocation addend.
 *
 * The one structural difference from the 32-bit shapes: x86_64 relocates with
 * SHT_RELA, so .rela.dyn/.rela.plt carry an explicit addend — the loaders read
 * it from the entry instead of from the text being patched. */

#include <stdint.h>

typedef struct {
    uint32_t st_name;
    uint8_t  st_info;
    uint8_t  st_other;
    uint16_t st_shndx;
    uint64_t st_value;
    uint64_t st_size;
} Elf64_Sym;

typedef struct {
    uint32_t p_type;
    uint32_t p_flags;
    uint64_t p_offset;
    uint64_t p_vaddr;
    uint64_t p_paddr;
    uint64_t p_filesz;
    uint64_t p_memsz;
    uint64_t p_align;
} Elf64_Phdr;

typedef struct {
    int64_t d_tag;
    union {
        uint64_t d_val;
        uint64_t d_ptr;
    } d_un;
} Elf64_Dyn;

typedef struct {
    uint64_t r_offset;
    uint64_t r_info;
} Elf64_Rel;

typedef struct {
    uint64_t r_offset;
    uint64_t r_info;
    int64_t  r_addend;
} Elf64_Rela;

/* The program header table's own entry: everything we need is a fixed offset
 * except these three, which the loader reads directly. */
typedef struct {
    uint8_t  e_ident[16];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint64_t e_entry;
    uint64_t e_phoff;
    uint64_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} Elf64_Ehdr;

/* x86_64 packs the symbol index and the type into one 64-bit r_info word: the
 * type in the low half, the index above it.  st_info keeps the nibble pair. */
#define ELF64_R_SYM(i)   ((uint32_t)((i) >> 32))
#define ELF64_R_TYPE(i)  ((uint32_t)(i))
#define ELF64_ST_BIND(i) ((i) >> 4)
#define ELF64_ST_TYPE(i) ((i) & 0xf)

#define ELF_EI_MAGIC0 0x7Fu          /* e_ident[0] */
#define ELF_EI_MAGIC1 'E'
#define ELF_EI_MAGIC2 'L'
#define ELF_EI_MAGIC3 'F'

#define EM_X86_64    62              /* e_machine of every image CactOS loads */
#define ET_EXEC      2
#define ET_DYN       3

#define PT_LOAD      1
#define PT_DYNAMIC   2
#define PT_INTERP    3
#define PT_TLS       7

#define DT_NULL      0
#define DT_NEEDED    1
#define DT_PLTRELSZ  2
#define DT_PLTGOT    3
#define DT_HASH      4
#define DT_STRTAB    5
#define DT_SYMTAB    6
#define DT_RELA      7
#define DT_RELASZ    8
#define DT_RELAENT   9
#define DT_STRSZ     10
#define DT_SYMENT    11
#define DT_INIT      12
#define DT_FINI      13
#define DT_SONAME    14
#define DT_REL       17
#define DT_RELSZ     18
#define DT_RELENT    19
#define DT_PLTREL    20
#define DT_JMPREL    23
#define DT_INIT_ARRAY    25
#define DT_FINI_ARRAY    26
#define DT_INIT_ARRAYSZ  27
#define DT_FINI_ARRAYSZ  28
#define DT_GNU_HASH  0x6ffffef5

#define R_X86_64_NONE       0
#define R_X86_64_64         1        /* S + A */
#define R_X86_64_PC32       2        /* S + A - P */
#define R_X86_64_COPY       5
#define R_X86_64_GLOB_DAT   6
#define R_X86_64_JUMP_SLOT  7
#define R_X86_64_RELATIVE   8        /* B + A, B = 0 */
#define R_X86_64_32        10
#define R_X86_64_32S       11
#define R_X86_64_PC64      24

/* x86_64 TLS relocations.  GNU ld reduces every static access to
 * R_X86_64_TPOFF64, whether the code is a program's own local-exec access or the
 * GOT slot an initial-exec access loads and adds to %fs:0; the loaders resolve
 * those.  General-dynamic comes through as the pair R_X86_64_DTPMOD64 /
 * _DTPOFF64 (the module id and the offset __tls_get_addr() adds), which the
 * loaders also resolve.  The rest name a model neither loader implements, and
 * they are reported rather than resolved wrongly. */
#define R_X86_64_DTPMOD64   16
#define R_X86_64_DTPOFF64   17
#define R_X86_64_TPOFF64    18
#define R_X86_64_TLSGD      19
#define R_X86_64_TLSLD      20
#define R_X86_64_DTPOFF32   21
#define R_X86_64_GOTTPOFF   22
#define R_X86_64_TPOFF32    23

#define SHN_UNDEF  0
#define STB_GLOBAL 1
#define STB_WEAK   2
#define STT_TLS    6

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
