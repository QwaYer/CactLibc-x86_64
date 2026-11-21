#include "syscall.h"
#include "unistd.h"
#include <stdarg.h>
#include <stdint.h>

/* The one non-inline entry point: <syscall.h> carries the same instruction for
 * the fixed-arity __syscallN forms, and this is what a caller uses when the
 * number is only known at run time.  A leaf function, so it is also the case
 * that exercises the red-zone question — see the note in <syscall.h>.
 *
 * Variadic, like POSIX's syscall(2): a C++ caller passes a pointer where the
 * kernel wants a word and only `...` accepts that without a cast (LLVM's
 * TargetParser/Host.cpp does exactly this).  CactOS's kernel takes three
 * arguments, so three are read — the same fixed set glibc's entry reads — and
 * a caller that passes fewer leaves the unused words undefined, which is the
 * caller's business (every in-tree caller passes three). */
intptr_t syscall(int num, ...) {
    /* SYS_gettid has no kernel entry (see <syscall.h>): CactOS serves the tid
     * through /proc/self, so answer the Linux-shaped call here — in userspace,
     * which is where that data lives on this system. */
    if (num == SYS_gettid)
        return (intptr_t)gettid();

    va_list ap;
    va_start(ap, num);
    uintptr_t p1 = va_arg(ap, uintptr_t);
    uintptr_t p2 = va_arg(ap, uintptr_t);
    uintptr_t p3 = va_arg(ap, uintptr_t);
    va_end(ap);

    intptr_t ret;
    __asm__ volatile (
        "syscall"
        : "=a"(ret)
        : "a"(num), "D"(p1), "S"(p2), "d"(p3)
        : "rcx", "r11", "memory"
    );
    return ret;
}
