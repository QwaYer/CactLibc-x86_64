#ifndef _SYS_AUXV_H
#define _SYS_AUXV_H

/* C linkage for the C++ consumers (libc++, LLVM, Mesa) that include this
 * header; a mangled reference would match no CactLibc export. */
#ifdef __cplusplus
extern "C" {
#endif

#include "elf.h"   /* the AT_* keys */

/* getauxval(3).  CactOS's kernel hands a program no auxiliary vector (the exec
 * entry carries argc/argv/envp and nothing after), so no AT_* value exists:
 * this reports ENOENT for every key, which is glibc's answer for a key that is
 * not present.  x86 uses CPUID rather than AT_HWCAP, so nothing a CactOS
 * program needs comes from here; the header is for the source that includes it
 * unconditionally. */
unsigned long getauxval(unsigned long type);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* _SYS_AUXV_H */
