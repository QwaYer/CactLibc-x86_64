#include "syscall.h"

/* The one non-inline entry point: <syscall.h> carries the same instruction for
 * the fixed-arity __syscallN forms, and this is what a caller uses when the
 * number is only known at run time.  A leaf function, so it is also the case
 * that exercises the red-zone question — see the note in <syscall.h>. */
intptr_t syscall(int num, uintptr_t p1, uintptr_t p2, uintptr_t p3) {
    intptr_t ret;
    __asm__ volatile (
        "syscall"
        : "=a"(ret)
        : "a"(num), "D"(p1), "S"(p2), "d"(p3)
        : "rcx", "r11", "memory"
    );
    return ret;
}
