#ifndef _SETJMP_H
#define _SETJMP_H

/* The context setjmp() saves and longjmp() restores: the callee-saved
 * registers the SysV AMD64 ABI names (rbx, rbp, r12-r15) plus the stack
 * pointer and the return address, one word each — see src/setjmp.S. */
typedef unsigned long jmp_buf[8];

int setjmp(jmp_buf env);
void longjmp(jmp_buf env, int val);

#endif
