.section .text
.global _start
.type _start, @function

_start:
    /* The kernel leaves rsp 16-byte aligned and pointing at the CactOS
     * argc/argv/envp triple (Cact/kernel/proc/sched/src/task_exec.rs), so a
     * `call` made from here already satisfies the SysV alignment rule.  rsp is
     * callee-saved territory only through rbp: keep the triple's address there,
     * because nothing below may assume rsp still points at it. */
    movq    %rsp, %rbp

    /* Seed `environ` from the exec-provided envp array before main(). */
    movq    16(%rbp), %rdi      /* envp */
    testq   %rdi, %rdi
    jz      1f
    call    __cact_init_env@PLT
1:
    /* Install this thread's static TLS block (__thread support) before any
     * program code can touch a TLS variable: the compiler emits plain
     * `%fs:...` accesses, so the %fs base has to be right already.  A program
     * with no TLS has an empty layout and this returns immediately. */
    call    __cact_tls_start@PLT

    movq    (%rbp),  %rdi       /* argc */
    movq    8(%rbp), %rsi       /* argv */
    movq    16(%rbp), %rdx      /* envp */
    call    main@PLT

    movl    %eax, %edi          /* exit(main(...)) */
    call    exit@PLT
    hlt

    /* crtbegin's __dso_handle.  clang emits a hidden reference to it in every
     * object that has a namespace-scope object with a destructor, to hand
     * __cxa_atexit() the image that registered the handler; hidden means the
     * definition has to live in the same image.  A program gets one from here,
     * a .so carries its own (crtbeginS.o defines it the same way: hidden and
     * self-referential). */
    .section .data,"aw",@progbits
    .hidden __dso_handle
    .globl  __dso_handle
    .type   __dso_handle, @object
    .align  8
__dso_handle:
    .quad   __dso_handle
    .size   __dso_handle, 8

.section .note.GNU-stack,"",@progbits
