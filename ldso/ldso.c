/* CactOS userspace dynamic linker bootstrap (ld.so).

 * Compiled as a freestanding fixed-address x86_64 executable, no libc.  It is
 * loaded by the kernel as the PT_INTERP interpreter; the kernel maps the main
 * image and ld.so itself but performs no relocations.  This component:
 *   1. parses auxv (AT_PHDR/AT_ENTRY/...),
 *   2. maps the shared libraries from DT_NEEDED into their fixed addresses,
 *   3. applies R_X86_64_* relocations to libc and the main image, then
 *   4. returns the main entry point to _start, which restores the original
 *      stack pointer and jumps there.
 *
 * CactOS policy: every image is mapped exactly at its link-time address, so
 * symbol values / relocation addends are already absolute and R_X86_64_RELATIVE
 * is just the stored addend (B + A with bias 0).
 *
 * x86_64 relocates with SHT_RELA, so .rela.dyn/.rela.plt — not .rel/.plt — are
 * where an object's dynamic relocations live; both shapes are handled below
 * because the same code serves whatever a toolchain emits. */

#include "ldso.h"

typedef struct {
    unsigned long phdr;
    unsigned long phent;
    unsigned long phnum;
    unsigned long pagesz;
    unsigned long base;
    unsigned long entry;
} auxv_t;

typedef struct {
    const char* name;      /* soname, "<main>" for the executable */
    unsigned long dyn;     /* PT_DYNAMIC vaddr (in memory) */
    unsigned long dynsz;   /* PT_DYNAMIC p_filesz */
    Elf64_Sym*  symtab;
    u32         symcount;
    char*       strtab;
    unsigned long strsz;
    /* PT_TLS (all zero when the object has no TLS) */
    u32         tls_vaddr, tls_filesz, tls_memsz, tls_align;
    u32         tls_off;   /* its offset in the static block (see tls_publish) */
    u32         tls_modid; /* its TLS module id (for general-dynamic relocations) */
    /* a copy of its program headers (the ELF header is not mapped, so they are
     * not readable after the fact) and the phdr count */
    unsigned long phdr;
    u32         phnum;
    /* the span the PT_LOADs cover, so a later dlopen() can refuse to map over it */
    unsigned long base, size;
    /* The relocation sections, kept so the TLS pass can re-walk them once
     * every object is loaded and the block layout is known. */
    Elf64_Rel*  rel;  unsigned long rel_sz,  rel_ent;
    Elf64_Rela* rela; unsigned long rela_sz, rela_ent;
} ldobj_t;

/* Raw syscall shim.  ABI: RAX=num, RDI=a1, RSI=a2, RDX=a3 (SYSCALL). */
__attribute__((noinline))
long ldso_sc(unsigned long num, unsigned long a1, unsigned long a2, unsigned long a3)
{
    long ret;
    __asm__ volatile(
        "syscall"
        : "=a"(ret)
        : "a"(num), "D"(a1), "S"(a2), "d"(a3)
        : "rcx", "r11", "memory");
    return ret;
}

void ldso_die(void)
{
    for (;;) ldso_sc(SYS_EXIT, 1, 0, 0);
}

/* ── minimal memory / string helpers ─────────────────────────────────── */

static void ld_memcpy(void* dst, const void* src, unsigned long n)
{
    u8* d = (u8*)dst;
    const u8* s = (const u8*)src;
    while (n--) *d++ = *s++;
}

static void ld_memset(void* dst, int v, unsigned long n)
{
    u8* d = (u8*)dst;
    while (n--) *d++ = (u8)v;
}

static int ld_strlen(const char* s)
{
    int n = 0;
    while (s[n]) n++;
    return n;
}

static int ld_strcmp(const char* a, const char* b)
{
    while (*a && *a == *b) { a++; b++; }
    return (unsigned char)*a - (unsigned char)*b;
}

/* ── debug output (fd 1) ─────────────────────────────────────────────── */

static void ld_dbg(const char* s)
{
    int n = ld_strlen(s);
    ldso_sc(SYS_WRITE, 1, (unsigned long)s, (unsigned long)n);
}

/* ── object table ────────────────────────────────────────────────────── */

static ldobj_t g_objs[LDSO_MAX_OBJS];
static int     g_nobj;

/* Program headers, copied out of the file scratch buffer as each object is
 * loaded: a CactOS image's ELF header is not mapped (its first PT_LOAD starts
 * at file offset 0x1000), so dl_iterate_phdr has nothing else to point at. */
#define DL_MAX_PHDRS CACT_DL_MAX_PHDRS
static Elf64_Phdr g_phdrs[LDSO_MAX_OBJS * DL_MAX_PHDRS];

static ldobj_t* obj_find_by_name(const char* name)
{
    int i;
    for (i = 0; i < g_nobj; i++) {
        if (ld_strcmp(g_objs[i].name, name) == 0) return &g_objs[i];
    }
    return 0;
}

static u32 symcount_from_hash(unsigned long hash, unsigned long gnu_hash, Elf64_Sym* symtab)
{
    u32 n = 0;
    if (!symtab) return 0;
    if (hash) {
        n = ((u32*)hash)[1];                     /* SYSV: nchain */
        return n;
    }
    if (gnu_hash) {
        u32* gh = (u32*)gnu_hash;
        u32 nbuckets   = gh[0];
        u32 symoffset  = gh[1];
        u32 bloom_size = gh[2];
        if (bloom_size > 1024) bloom_size = 1024;
        if (nbuckets == 0) return 0;
        {
            u32* buckets = gh + 4 + bloom_size;
            u32* chains  = buckets + nbuckets;
            u32  max_b   = 0, i;
            for (i = 0; i < nbuckets; i++)
                if (buckets[i] > max_b) max_b = buckets[i];
            if (max_b < symoffset) return symoffset;
            {
                u32 ci = max_b - symoffset;
                u32 guard;
                for (guard = 0; guard < 8192; guard++, ci++) {
                    if (chains[ci] & 1u) return symoffset + ci + 1;
                }
            }
            return symoffset + 8192;
        }
    }
    return 4096;                                /* kernel fallback */
}

static int load_library(const char* name);

static unsigned long resolve_symbol(const char* name)
{
    int o;
    for (o = 0; o < g_nobj; o++) {
        ldobj_t* ob = &g_objs[o];
        u32 j;
        if (!ob->symtab || !ob->strtab) continue;
        for (j = 1; j < ob->symcount; j++) {
            Elf64_Sym* sym = &ob->symtab[j];
            const char* sn;
            int bind;
            if (sym->st_shndx == SHN_UNDEF) continue;
            bind = ELF64_ST_BIND(sym->st_info);
            if (bind != 1 /* STB_GLOBAL */ && bind != STB_WEAK) continue;
            if (sym->st_value == 0) continue;
            if (sym->st_name >= ob->strsz) continue;
            sn = ob->strtab + sym->st_name;
            if (ld_strcmp(sn, name) == 0) return (unsigned long)sym->st_value;
        }
    }
    return 0;
}

/* ── relocation application ──────────────────────────────────────────── */

/* The relocation's symbol value: 0 for a symbol this object does not define and
 * that no loaded object exports, which is exactly what a weak undefined symbol
 * has to come out as. */
static unsigned long reloc_symbol_value(ldobj_t* ob, u32 sym_idx)
{
    if (sym_idx == 0 || !ob->symtab || !ob->strtab) return 0;
    if (sym_idx >= ob->symcount) return 0;
    {
        Elf64_Sym* sym = &ob->symtab[sym_idx];
        if (sym->st_shndx == SHN_UNDEF) {
            const char* sname;
            if (sym->st_name >= ob->strsz) return 0;
            sname = ob->strtab + sym->st_name;
            return resolve_symbol(sname);
        }
        return (unsigned long)sym->st_value;
    }
}

/* SHT_REL: the addend is what the instruction or word already holds. */
static void apply_rel(ldobj_t* ob, Elf64_Rel* rel)
{
    unsigned long sym_idx  = ELF64_R_SYM(rel->r_info);
    u32           rel_type = ELF64_R_TYPE(rel->r_info);
    unsigned long* target  = (unsigned long*)rel->r_offset;
    unsigned long S = reloc_symbol_value(ob, (u32)sym_idx);
    unsigned long A, P = rel->r_offset;

    switch (rel_type) {
    case R_X86_64_NONE:
        break;
    case R_X86_64_64:         /* S + A */
        A = *target;
        *target = S + A;
        break;
    case R_X86_64_PC32:       /* S + A - P */
        A = *target;
        *target = S + A - P;
        break;
    case R_X86_64_GLOB_DAT:
    case R_X86_64_JUMP_SLOT:  /* S */
        *target = S;
        break;
    case R_X86_64_RELATIVE:   /* B + A, B = 0 (fixed-address images) */
        break;
    case R_X86_64_COPY:
        if (S && sym_idx < ob->symcount) {
            Elf64_Sym* sym = &ob->symtab[sym_idx];
            if (sym->st_size > 0 && sym->st_size <= 65536)
                ld_memcpy(target, (void*)S, (unsigned long)sym->st_size);
        }
        break;
    default:
        break;
    }
}

/* SHT_RELA: the addend travels in the entry. */
static void apply_rela(ldobj_t* ob, Elf64_Rela* rela)
{
    unsigned long sym_idx  = ELF64_R_SYM(rela->r_info);
    u32           rel_type = ELF64_R_TYPE(rela->r_info);
    unsigned long* target  = (unsigned long*)rela->r_offset;
    unsigned long S = reloc_symbol_value(ob, (u32)sym_idx);
    unsigned long A = (unsigned long)rela->r_addend;
    unsigned long P = rela->r_offset;

    switch (rel_type) {
    case R_X86_64_NONE:
        break;
    case R_X86_64_64:
        *target = S + A;
        break;
    case R_X86_64_PC32:
        *target = S + A - P;
        break;
    case R_X86_64_GLOB_DAT:
    case R_X86_64_JUMP_SLOT:
        *target = S;
        break;
    case R_X86_64_RELATIVE:
        /* On i386 this was a no-op (the addend was already in the word); with
         * RELA the entry carries it and the word has to be written. */
        *target = A;
        break;
    case R_X86_64_COPY:
        if (S && sym_idx < ob->symcount) {
            Elf64_Sym* sym = &ob->symtab[sym_idx];
            if (sym->st_size > 0 && sym->st_size <= 65536)
                ld_memcpy(target, (void*)S, (unsigned long)sym->st_size);
        }
        break;
    default:
        break;
    }
}

/* Parse the object's .dynamic (in memory), register it, recursively load its
 * DT_NEEDED deps, then apply its own relocations. */
static void process_object(ldobj_t* ob)
{
    Elf64_Dyn* dyn       = (Elf64_Dyn*)ob->dyn;
    unsigned long max_ent = ob->dynsz / sizeof(Elf64_Dyn);
    Elf64_Sym* symtab    = 0;
    char*      strtab    = 0;
    unsigned long strsz  = 0;
    unsigned long hash   = 0;
    unsigned long gnu_hash = 0;
    Elf64_Rel* rel       = 0;
    unsigned long rel_sz = 0, rel_ent = sizeof(Elf64_Rel);
    Elf64_Rela* rela     = 0;
    unsigned long rela_sz = 0, rela_ent = sizeof(Elf64_Rela);
    Elf64_Rel* jmprel    = 0;
    unsigned long jmprel_sz = 0;
    unsigned long pltrel = DT_REL;
    u32        needed[LDSO_MAX_NEEDED];
    int        nneed = 0;
    unsigned long i;

    for (i = 0; i < max_ent; i++) {
        Elf64_Dyn* d = &dyn[i];
        if (d->d_tag == DT_NULL) break;
        switch (d->d_tag) {
        case DT_SYMTAB:  symtab = (Elf64_Sym*)d->d_un.d_ptr; break;
        case DT_STRTAB:  strtab = (char*)d->d_un.d_ptr;       break;
        case DT_STRSZ:   strsz  = d->d_un.d_val;              break;
        case DT_HASH:    hash   = d->d_un.d_ptr;              break;
        case DT_GNU_HASH: gnu_hash = d->d_un.d_ptr;           break;
        case DT_REL:     rel    = (Elf64_Rel*)d->d_un.d_ptr;  break;
        case DT_RELSZ:   rel_sz = d->d_un.d_val;              break;
        case DT_RELENT:  rel_ent = d->d_un.d_val;             break;
        case DT_RELA:    rela   = (Elf64_Rela*)d->d_un.d_ptr; break;
        case DT_RELASZ:  rela_sz = d->d_un.d_val;             break;
        case DT_RELAENT: rela_ent = d->d_un.d_val;            break;
        case DT_JMPREL:  jmprel = (Elf64_Rel*)d->d_un.d_ptr;  break;
        case DT_PLTRELSZ: jmprel_sz = d->d_un.d_val;          break;
        case DT_PLTREL:  pltrel = d->d_un.d_val;              break;
        case DT_NEEDED:
            if (nneed < LDSO_MAX_NEEDED) needed[nneed++] = (u32)d->d_un.d_val;
            break;
        default: break;
        }
    }

    ob->symtab    = symtab;
    ob->strtab    = strtab;
    ob->strsz     = strsz;
    ob->symcount  = symcount_from_hash(hash, gnu_hash, symtab);
    ob->rel       = rel;  ob->rel_sz  = rel_sz;  ob->rel_ent  = rel_ent;
    ob->rela      = rela; ob->rela_sz = rela_sz; ob->rela_ent = rela_ent;

    /* dependencies (their relocations run first) */
    for (i = 0; i < (unsigned long)nneed; i++) {
        const char* name;
        if (!strtab || needed[i] >= strsz) continue;
        name = strtab + needed[i];
        if (!obj_find_by_name(name)) load_library(name);
    }

    if (rel && rel_sz && rel_ent) {
        unsigned long n = rel_sz / rel_ent;
        for (i = 0; i < n; i++)
            apply_rel(ob, (Elf64_Rel*)((u8*)rel + i * rel_ent));
    }
    if (rela && rela_sz && rela_ent) {
        unsigned long n = rela_sz / rela_ent;
        for (i = 0; i < n; i++)
            apply_rela(ob, (Elf64_Rela*)((u8*)rela + i * rela_ent));
    }
    if (jmprel && jmprel_sz) {
        if (pltrel == DT_RELA) {
            unsigned long n = jmprel_sz / sizeof(Elf64_Rela);
            for (i = 0; i < n; i++)
                apply_rela(ob, (Elf64_Rela*)((u8*)jmprel + i * sizeof(Elf64_Rela)));
        } else {
            unsigned long n = jmprel_sz / sizeof(Elf64_Rel);
            for (i = 0; i < n; i++)
                apply_rel(ob, (Elf64_Rel*)((u8*)jmprel + i * sizeof(Elf64_Rel)));
        }
    }
}

/* ── file loading helpers ────────────────────────────────────────────── */

static int read_file_full(const char* path, u8* buf, u32 cap)
{
    int fd = (int)ldso_sc(SYS_OPEN, (unsigned long)path, 0, 0);
    u32  off = 0;
    if (fd < 0) return -1;
    for (;;) {
        long n;
        if (off >= cap) break;
        n = ldso_sc(SYS_READ, (unsigned long)fd, (unsigned long)(buf + off), cap - off);
        if (n <= 0) break;
        off += (u32)n;
    }
    ldso_sc(SYS_CLOSE, (unsigned long)fd, 0, 0);
    return (off >= cap) ? -2 : (int)off;
}

/* The mmap argument block the kernel reads (mmap_args_t): the three sizes are
 * 32-bit because the whole user half — every image's fixed address included —
 * is below 4 GiB. */
typedef struct {
    u32  addr;
    u32  length;
    int  prot;
    int  flags;
    int  fd;
    u32  offset;
} ld_mmap_args_t;

static unsigned long map_segment(unsigned long vaddr, unsigned long memsz)
{
    unsigned long start = vaddr & ~0xFFFul;
    unsigned long end   = (vaddr + memsz + 0xFFFul) & ~0xFFFul;
    ld_mmap_args_t args;
    long r;
    if (end <= start) return 0;
    args.addr   = (u32)start;
    args.length = (u32)(end - start);
    args.prot   = PROT_READ | PROT_WRITE | PROT_EXEC;
    args.flags  = MAP_PRIVATE | MAP_FIXED | MAP_ANON;
    args.fd     = -1;
    args.offset = 0;
    r = ldso_sc(SYS_MMAP, (unsigned long)&args, 0, 0);
    return (r == (long)-1) ? 0 : start;
}

/* Map libc/libfoo.so at its fixed link-time addresses; returns 0 on failure. */
static int load_library(const char* name)
{
    char path[SO_PATH_MAX];
    int  len;
    u8*  buf;
    u32  cap;
    int  total;
    Elf64_Ehdr* eh;
    unsigned long phoff, phentsz;
    u16  phnum;
    int  i;
    Elf64_Dyn* dyn = 0;
    unsigned long dynsz = 0;
    u32        tls_vaddr = 0, tls_filesz = 0, tls_memsz = 0, tls_align = 4;
    unsigned long span_lo = 0, span_hi = 0;

    if (g_nobj >= LDSO_MAX_OBJS) return -1;

    len = ld_strlen(name);
    if (len + 10 > SO_PATH_MAX) return -1;
    ld_memcpy(path, "/usr/lib/", 9);
    ld_memcpy(path + 9, name, (unsigned long)len + 1);

    cap = LDSO_SCRATCH;
    {
        ld_mmap_args_t sargs;
        sargs.addr = LDSO_SCRATCH_BASE; sargs.length = cap;
        sargs.prot = PROT_READ | PROT_WRITE;
        sargs.flags = MAP_PRIVATE | MAP_ANON | MAP_FIXED;
        sargs.fd = -1; sargs.offset = 0;
        buf = (u8*)ldso_sc(SYS_MMAP, (unsigned long)&sargs, 0, 0);
    }
    if (buf == (u8*)-1) return -1;

    total = read_file_full(path, buf, cap);
    if (total <= 0) {
        ldso_sc(SYS_MUNMAP, (unsigned long)buf, cap, 0);
        ld_dbg("[ld.so] cannot read "); ld_dbg(path); ld_dbg("\n");
        return -1;
    }

    eh = (Elf64_Ehdr*)buf;
    if (eh->e_ident[0] != ELF_EI_MAGIC0 || eh->e_ident[1] != ELF_EI_MAGIC1 ||
        eh->e_ident[2] != ELF_EI_MAGIC2 || eh->e_ident[3] != ELF_EI_MAGIC3) {
        ldso_sc(SYS_MUNMAP, (unsigned long)buf, cap, 0);
        ld_dbg("[ld.so] bad magic: "); ld_dbg(path); ld_dbg("\n");
        return -1;
    }
    /* A 32-bit image here would be mapped and then relocated with the wrong
     * rules; every CactOS image is x86_64, so say so instead. */
    if (eh->e_machine != EM_X86_64) {
        ldso_sc(SYS_MUNMAP, (unsigned long)buf, cap, 0);
        ld_dbg("[ld.so] not an x86_64 image: "); ld_dbg(path); ld_dbg("\n");
        return -1;
    }
    phoff   = eh->e_phoff;
    phnum   = eh->e_phnum;
    phentsz = eh->e_phentsize;

    if (phentsz < sizeof(Elf64_Phdr) || phnum == 0 ||
        phoff + (unsigned long)phnum * phentsz > (unsigned long)total) {
        ldso_sc(SYS_MUNMAP, (unsigned long)buf, cap, 0);
        return -1;
    }

    /* two passes: map PT_LOAD, remember PT_DYNAMIC */
    for (i = 0; i < phnum; i++) {
        Elf64_Phdr* ph = (Elf64_Phdr*)(buf + phoff + (unsigned long)i * phentsz);
        if (ph->p_type == PT_TLS) {
            tls_vaddr  = (u32)ph->p_vaddr;
            tls_filesz = (u32)ph->p_filesz;
            tls_memsz  = (u32)ph->p_memsz;
            tls_align  = (u32)ph->p_align;
            continue;
        }
        if (ph->p_type != PT_LOAD || ph->p_memsz == 0) continue;
        {
            unsigned long lo = ph->p_vaddr & ~0xFFFul;
            unsigned long hi = (ph->p_vaddr + ph->p_memsz + 0xFFFul) & ~0xFFFul;
            if (!span_lo || lo < span_lo) span_lo = lo;
            if (hi > span_hi) span_hi = hi;
        }
        if (!map_segment(ph->p_vaddr, ph->p_memsz)) {
            ldso_sc(SYS_MUNMAP, (unsigned long)buf, cap, 0);
            ld_dbg("[ld.so] mmap segment failed: "); ld_dbg(path); ld_dbg("\n");
            return -1;
        }
    }
    for (i = 0; i < phnum; i++) {
        Elf64_Phdr* ph = (Elf64_Phdr*)(buf + phoff + (unsigned long)i * phentsz);
        if (ph->p_type == PT_DYNAMIC) {
            dyn = (Elf64_Dyn*)ph->p_vaddr;
            dynsz = ph->p_filesz;
            break;
        }
    }

    /* copy segment file bytes into the fresh zeroed mappings */
    for (i = 0; i < phnum; i++) {
        Elf64_Phdr* ph = (Elf64_Phdr*)(buf + phoff + (unsigned long)i * phentsz);
        unsigned long seg_va;
        if (ph->p_type != PT_LOAD || ph->p_filesz == 0) continue;
        if (ph->p_offset + ph->p_filesz > (unsigned long)total) continue;
        seg_va = ph->p_vaddr & ~0xFFFul;
        ld_memcpy((void*)(seg_va + (ph->p_vaddr - seg_va)),
                  buf + ph->p_offset, ph->p_filesz);
    }

    /* Keep a copy of the program headers while the scratch buffer is still
     * there: the ELF header of a CactOS image is not mapped (its first PT_LOAD
     * starts at file offset 0x1000), so dl_iterate_phdr has nothing else to
     * point a caller at.  The slot is the index this object is about to take. */
    if (g_nobj < LDSO_MAX_OBJS) {
        u32 n = phnum < DL_MAX_PHDRS ? phnum : DL_MAX_PHDRS;
        u32 slot = (u32)g_nobj;
        for (i = 0; i < (int)n; i++)
            ld_memcpy(&g_phdrs[slot * DL_MAX_PHDRS + i],
                      buf + phoff + (unsigned long)i * phentsz, sizeof(Elf64_Phdr));
    }

    ldso_sc(SYS_MUNMAP, (unsigned long)buf, cap, 0);

    if (g_nobj < LDSO_MAX_OBJS) {
        ldobj_t* ob = &g_objs[g_nobj++];
        ob->name   = name;
        ob->dyn    = (unsigned long)dyn;
        ob->dynsz  = dynsz;
        ob->symtab = 0; ob->strtab = 0; ob->strsz = 0; ob->symcount = 0;
        ob->rel = 0; ob->rel_sz = 0; ob->rel_ent = 0;
        ob->rela = 0; ob->rela_sz = 0; ob->rela_ent = 0;
        ob->tls_vaddr = tls_vaddr; ob->tls_filesz = tls_filesz;
        ob->tls_memsz = tls_memsz; ob->tls_align  = tls_align;
        ob->tls_off   = 0;
        ob->base      = span_lo;
        ob->size      = span_hi - span_lo;
        ob->phdr      = (unsigned long)&g_phdrs[(unsigned long)(g_nobj - 1) * DL_MAX_PHDRS];
        ob->phnum     = phnum < DL_MAX_PHDRS ? phnum : DL_MAX_PHDRS;
        process_object(ob);
        return 0;
    }
    return -1;
}

/* ── auxv parsing ────────────────────────────────────────────────────── */

static int parse_auxv(unsigned long* stackp, auxv_t* av)
{
    /* CactOS exec stack: argc, argv_arr, envp_arr — one 64-bit word each, at
     * the 16-byte-aligned stack the kernel hands over. */
    unsigned long* argv = (unsigned long*)stackp[1];
    unsigned long* envp = (unsigned long*)stackp[2];
    unsigned long* aux;
    int  i = 0;

    ld_memset(av, 0, sizeof(*av));

    if (envp) {
        while (envp[i]) i++;
        aux = envp + i + 1;
    } else if (argv) {
        while (argv[i]) i++;
        aux = argv + i + 1;
    } else {
        return -1;
    }

    while (aux[0] != AT_NULL) {
        switch (aux[0]) {
        case AT_PHDR:   av->phdr   = aux[1]; break;
        case AT_PHENT:  av->phent  = aux[1]; break;
        case AT_PHNUM:  av->phnum  = aux[1]; break;
        case AT_PAGESZ: av->pagesz = aux[1]; break;
        case AT_BASE:   av->base   = aux[1]; break;
        case AT_ENTRY:  av->entry  = aux[1]; break;
        default: break;
        }
        aux += 2;
    }
    return (av->phdr && av->phnum && av->phent) ? 0 : -1;
}

/* ── static TLS ──────────────────────────────────────────────────────────── */

static u32 g_tls_total;

/* One module per object that carries a PT_TLS, in load order, and hand the
 * resulting layout to the libc — the libc cannot see the program headers, and
 * it is the one that builds each thread's block (src/tls_static.c). */
static void tls_publish(void)
{
    cact_tls_layout_t lay;
    int i, n = 0;
    unsigned long addr;

    ld_memset(&lay, 0, sizeof(lay));
    for (i = 0; i < g_nobj && n < CACT_TLS_MAX_MODULES; i++) {
        ldobj_t* ob = &g_objs[i];
        ob->tls_off = 0;
        ob->tls_modid = 0;
        if (!ob->tls_memsz) continue;
        lay.mods[n].off    = 0;
        lay.mods[n].vaddr  = ob->tls_vaddr;
        lay.mods[n].filesz = ob->tls_filesz;
        lay.mods[n].memsz  = ob->tls_memsz;
        lay.mods[n].align  = ob->tls_align;
        n++;
    }
    /* Room for the objects that will show up later: a dlopen()'d plugin's own
     * __thread has to land inside every thread's block, and the block is made
     * once, before _start.  It is published even when nothing here uses
     * __thread, so a program with none of its own still has %fs pointing at a
     * block the moment it loads something that does. */
    g_tls_total = cact_tls_plan_reserved(&lay, (u32)n, CACT_TLS_RESERVE);

    /* the same walk, so each object learns its own offset and module id */
    n = 0;
    for (i = 0; i < g_nobj && n < CACT_TLS_MAX_MODULES; i++) {
        if (!g_objs[i].tls_memsz) continue;
        g_objs[i].tls_off   = lay.mods[n].off;
        g_objs[i].tls_modid = lay.mods[n].id;
        n++;
    }

    addr = resolve_symbol("cact_tls_layout");
    if (addr) ld_memcpy((void*)addr, &lay, sizeof(lay));
    else ld_dbg("[ld.so] libc has no cact_tls_layout\n");
}

/* Resolve what the linker left: after linking, static TLS access to a variable
 * is an R_X86_64_TPOFF64 (or the GOT slot an initial-exec access loads and adds
 * to %fs:0, which is R_X86_64_GOTTPOFF) against a symbol whose st_value is that
 * variable's offset within its module's TLS image, and the displacement to store
 * is the distance from the thread pointer (the TCB, which sits just past the
 * block) down to the variable.  General-dynamic is the pair
 * R_X86_64_DTPMOD64/_DTPOFF64 (the module id and the offset __tls_get_addr()
 * adds), which only a shared object that is not initial-exec emits; the rest
 * name a model this ld.so does not implement, so they are reported rather than
 * silently left unresolved. */
static int is_static_tls_type(u32 t)
{
    return t == R_X86_64_TPOFF64 || t == R_X86_64_GOTTPOFF || t == R_X86_64_TPOFF32;
}

static void tls_relocate(void)
{
    int i;

    for (i = 0; i < g_nobj; i++) {
        ldobj_t* ob = &g_objs[i];
        unsigned long j, n;

        if (ob->rel && ob->rel_sz && ob->rel_ent) {
            n = ob->rel_sz / ob->rel_ent;
            for (j = 0; j < n; j++) {
                Elf64_Rel* r = (Elf64_Rel*)((u8*)ob->rel + j * ob->rel_ent);
                u32 t = ELF64_R_TYPE(r->r_info);
                u32 si = ELF64_R_SYM(r->r_info);
                unsigned long* target = (unsigned long*)r->r_offset;
                unsigned long S = 0;

                if (is_static_tls_type(t)) {
                    if (si && si < ob->symcount) S = (unsigned long)ob->symtab[si].st_value;
                    *target = ob->tls_off + S + *target - g_tls_total;
                    continue;
                }
                if (t == R_X86_64_DTPMOD64) {
                    /* General-dynamic's module-id half: the defining object's
                     * TLS id.  The program is -fPIE (local-exec) and no startup
                     * library carries __thread, so this only matters to an
                     * object that is neither — the same "assume it is the
                     * object's own symbol" rule the TPOFF case above uses. */
                    if (!ob->tls_memsz) {
                        ld_dbg("[ld.so] unsupported TLS relocation\n");
                        continue;
                    }
                    *target = ob->tls_modid + *target;
                    continue;
                }
                if (t == R_X86_64_DTPOFF64) {
                    if (si && si < ob->symcount) S = (unsigned long)ob->symtab[si].st_value;
                    *target = S + *target;
                    continue;
                }
                if (t >= R_X86_64_DTPMOD64 && t <= R_X86_64_TPOFF32)
                    ld_dbg("[ld.so] unsupported TLS relocation\n");
            }
        }
        if (ob->rela && ob->rela_sz && ob->rela_ent) {
            n = ob->rela_sz / ob->rela_ent;
            for (j = 0; j < n; j++) {
                Elf64_Rela* r = (Elf64_Rela*)((u8*)ob->rela + j * ob->rela_ent);
                u32 t = ELF64_R_TYPE(r->r_info);
                u32 si = ELF64_R_SYM(r->r_info);
                unsigned long* target = (unsigned long*)r->r_offset;
                unsigned long S = 0;

                if (is_static_tls_type(t)) {
                    if (si && si < ob->symcount) S = (unsigned long)ob->symtab[si].st_value;
                    *target = ob->tls_off + S + (unsigned long)r->r_addend - g_tls_total;
                    continue;
                }
                if (t == R_X86_64_DTPMOD64) {
                    if (!ob->tls_memsz) {
                        ld_dbg("[ld.so] unsupported TLS relocation\n");
                        continue;
                    }
                    *target = ob->tls_modid + (unsigned long)r->r_addend;
                    continue;
                }
                if (t == R_X86_64_DTPOFF64) {
                    if (si && si < ob->symcount) S = (unsigned long)ob->symtab[si].st_value;
                    *target = S + (unsigned long)r->r_addend;
                    continue;
                }
                if (t >= R_X86_64_DTPMOD64 && t <= R_X86_64_TPOFF32)
                    ld_dbg("[ld.so] unsupported TLS relocation\n");
            }
        }
    }
}

/* Hand the libc the list of objects already mapped, so dlopen() can reuse them
 * (clibc.so above all: every plugin's DT_NEEDED names it) and can refuse to map
 * a new object over one of them.  Pinned: dlclose() never unloads these. */
static void dl_publish(void)
{
    cact_dl_table_t t;
    unsigned long addr = resolve_symbol("cact_dl_table");
    int i;

    if (!addr) {
        ld_dbg("[ld.so] libc has no cact_dl_table\n");
        return;
    }
    t.count = 0;
    for (i = 0; i < g_nobj && t.count < CACT_DL_MAX_OBJECTS; i++) {
        ldobj_t* ob = &g_objs[i];
        cact_dl_object_t* o = &t.objs[t.count];
        o->name     = ob->name;
        o->dyn      = ob->dyn;
        o->dynsz    = ob->dynsz;
        o->symtab   = (unsigned long)ob->symtab;
        o->strtab   = (unsigned long)ob->strtab;
        o->symcount = ob->symcount;
        o->strsz    = ob->strsz;
        o->phdr     = ob->phdr;
        o->phnum    = ob->phnum;
        o->base     = ob->base;
        o->size     = ob->size;
        o->flags    = CACT_DL_PINNED;
        o->refs     = 1;
        o->tls_off  = ob->tls_memsz ? ob->tls_off : CACT_DL_NO_TLS;
        o->tls_modid = ob->tls_memsz ? ob->tls_modid : 0;
        t.count++;
    }
    ld_memcpy((void*)addr, &t, sizeof(t));
}

/* ── entry ───────────────────────────────────────────────────────────── */

unsigned long ldso_entry(unsigned long* stackp)
{
    auxv_t   av;
    Elf64_Phdr* ph;
    unsigned long main_dyn = 0, main_dynsz = 0;
    u32      main_tls_vaddr = 0, main_tls_filesz = 0, main_tls_memsz = 0, main_tls_align = 4;
    unsigned long main_lo = 0, main_hi = 0;
    ldobj_t* mainobj;
    unsigned long i;

    if (parse_auxv(stackp, &av) != 0) {
        ld_dbg("[ld.so] no auxv\n");
        return 0;
    }

    /* find main's PT_DYNAMIC (and PT_TLS) via the mapped phdr table */
    for (i = 0; i < av.phnum; i++) {
        ph = (Elf64_Phdr*)(av.phdr + i * av.phent);
        if (ph->p_type == PT_DYNAMIC) {
            main_dyn   = ph->p_vaddr;
            main_dynsz = ph->p_filesz;
        } else if (ph->p_type == PT_TLS) {
            main_tls_vaddr  = (u32)ph->p_vaddr;
            main_tls_filesz = (u32)ph->p_filesz;
            main_tls_memsz  = (u32)ph->p_memsz;
            main_tls_align  = (u32)ph->p_align;
        } else if (ph->p_type == PT_LOAD && ph->p_memsz) {
            unsigned long lo = ph->p_vaddr & ~0xFFFul;
            unsigned long hi = (ph->p_vaddr + ph->p_memsz + 0xFFFul) & ~0xFFFul;
            if (!main_lo || lo < main_lo) main_lo = lo;
            if (hi > main_hi) main_hi = hi;
        }
    }
    if (!main_dyn) {
        ld_dbg("[ld.so] no PT_DYNAMIC in main\n");
        return 0;
    }

    mainobj = &g_objs[g_nobj++];
    mainobj->name   = "<main>";
    mainobj->dyn    = main_dyn;
    mainobj->dynsz  = main_dynsz;
    mainobj->symtab = 0; mainobj->strtab = 0; mainobj->strsz = 0; mainobj->symcount = 0;
    mainobj->rel = 0; mainobj->rel_sz = 0; mainobj->rel_ent = 0;
    mainobj->rela = 0; mainobj->rela_sz = 0; mainobj->rela_ent = 0;
    mainobj->tls_vaddr = main_tls_vaddr; mainobj->tls_filesz = main_tls_filesz;
    mainobj->tls_memsz = main_tls_memsz; mainobj->tls_align  = main_tls_align;
    mainobj->tls_off   = 0;
    mainobj->base      = main_lo;
    mainobj->size      = main_hi - main_lo;
    /* The main's phdrs *are* mapped (AT_PHDR), so copy them out of there. */
    {
        u32 n = av.phnum < DL_MAX_PHDRS ? (u32)av.phnum : DL_MAX_PHDRS;
        for (i = 0; i < n; i++)
            ld_memcpy(&g_phdrs[i], (void*)(av.phdr + i * av.phent), sizeof(Elf64_Phdr));
        mainobj->phdr  = (unsigned long)&g_phdrs[0];
        mainobj->phnum = n;
    }

    process_object(mainobj);

    /* Every object is loaded now, so the static TLS block can be laid out and
     * the TLS relocations (which depend on its total size) resolved.  Both are
     * no-ops when nothing in the program uses __thread. */
    tls_publish();
    tls_relocate();

    /* What dlopen() has to know about the image it is starting inside. */
    dl_publish();

    return av.entry;
}
