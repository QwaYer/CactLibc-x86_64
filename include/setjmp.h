#ifndef _SETJMP_H
#define _SETJMP_H

/* C linkage for the C++ consumers (libc++, LLVM, Mesa) that include this
 * header; a mangled reference would match no CactLibc export. */
#ifdef __cplusplus
extern "C" {
#endif

/* The context setjmp() saves and longjmp() restores: the callee-saved
 * registers the SysV AMD64 ABI names (rbx, rbp, r12-r15) plus the stack
 * pointer and the return address, one word each — see src/setjmp.S. */
typedef unsigned long jmp_buf[8];

int setjmp(jmp_buf env);
void longjmp(jmp_buf env, int val);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
