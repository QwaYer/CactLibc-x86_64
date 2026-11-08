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

.section .note.GNU-stack,"",@progbits
