/* ld.so _start: save the original user stack pointer, run the C bootstrap,
 * then jump to the relocated main entry point on the same original stack. */

.section .text
.globl _start
.type _start, @function

_start:
    /* rbx = original stack pointer (points at argc).  The kernel leaves rsp
     * 16-byte aligned, and rbx is callee-saved, so it survives ldso_entry and
     * the `call` below needs no padding. */
    movq %rsp, %rbx
    movq %rbx, %rdi             /* arg1: pointer to argc */
    call ldso_entry
    movq %rbx, %rsp             /* restore original user stack */
    testq %rax, %rax
    jz 1f
    jmp *%rax                   /* enter the relocated main image */
1:
    call ldso_die               /* returns 0 from ldso_entry -> fatal */
2:
    hlt
    jmp 2b

.section .note.GNU-stack,"",@progbits
