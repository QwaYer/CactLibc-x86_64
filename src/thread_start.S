.section .text
.global __cact_thread_entry
.type __cact_thread_entry, @function

/* First instruction of every worker thread.
 *
 * The kernel enters a new thread with `iretq` (task_thread.rs builds a bare
 * IRETQ frame whose rip is the `entry` the libc handed over), which cannot set
 * an argument register — while the SysV AMD64 ABI passes the first argument in
 * %rdi.  So `entry` is this trampoline, not the caller's start_routine: the two
 * words at [rsp] are [start_routine][arg], laid down by pthread_create.
 *
 * rsp arrives 16-byte aligned (pthread_create rounds the stack top and pushes
 * exactly two words), and nothing here pushes before the call, so the alignment
 * rule for `call *%r11` already holds.
 *
 * The exit body is called directly rather than through the PLT: on x86_64 a
 * call needs no GOT register, and __cact_thread_exit_body has hidden visibility
 * precisely so this stays a link-time-resolved rel32 in the shared object too. */
__cact_thread_entry:
    movq    (%rsp), %r11        /* start_routine */
    movq    8(%rsp), %rdi       /* its argument */
    call    *%r11
    movq    %rax, %rdi          /* the start routine's return value */
    call    __cact_thread_exit_body
    hlt
.size __cact_thread_entry, . - __cact_thread_entry

.section .note.GNU-stack,"",@progbits
