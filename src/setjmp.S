/* setjmp/longjmp for x86_64.
 *
 * The SysV AMD64 ABI makes rbx, rbp and r12-r15 the callee-saved registers, so
 * that is the set a non-local jump has to put back, plus the stack pointer and
 * the return address.  jmp_buf is eight words (<setjmp.h>).
 *
 * setjmp returns 0; longjmp restores the context and returns `val`, or 1 when
 * `val` is 0 (a longjmp(buf, 0) must not look like setjmp's first return).
 */

.section .text
.global setjmp
.type setjmp, @function
setjmp:
    movq %rbx,  0(%rdi)
    movq %rbp,  8(%rdi)
    movq %r12, 16(%rdi)
    movq %r13, 24(%rdi)
    movq %r14, 32(%rdi)
    movq %r15, 40(%rdi)
    leaq 8(%rsp), %rax          /* rsp as it will be after this ret */
    movq %rax, 48(%rdi)
    movq (%rsp), %rax           /* the return address */
    movq %rax, 56(%rdi)
    xorl %eax, %eax
    ret

.global longjmp
.type longjmp, @function
longjmp:
    movq  0(%rdi), %rbx
    movq  8(%rdi), %rbp
    movq 16(%rdi), %r12
    movq 24(%rdi), %r13
    movq 32(%rdi), %r14
    movq 40(%rdi), %r15
    movl %esi, %eax
    testl %eax, %eax
    jne 1f
    movl $1, %eax               /* longjmp(buf, 0) returns 1 */
1:
    movq 56(%rdi), %r11         /* return address, read before rsp moves */
    movq 48(%rdi), %rsp
    jmp *%r11

.section .note.GNU-stack,"",@progbits
