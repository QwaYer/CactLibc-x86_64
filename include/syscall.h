#ifndef _SYSCALL_H
#define _SYSCALL_H

#include <stdint.h>

/*
 * Minimal syscall ABI (15 numbers). The numbers and their order must match:
 *   CactKernel-x86_64/Cact/kernel/core/syscall/syscalls.h
 * (syscall_num_t / SYSCALL_COUNT). Everything else goes through VFS nodes: see
 * ioctl_abi.h and the relay layer in src/nodeio.c.
 */

#define SYS_OPEN      0
#define SYS_CLOSE     1
#define SYS_READ      2
#define SYS_WRITE     3
#define SYS_IOCTL     4
#define SYS_POLL      5
#define SYS_FORK      6
#define SYS_EXEC      7
#define SYS_EXIT      8
#define SYS_WAITPID   9
#define SYS_BRK       10
#define SYS_MMAP      11
#define SYS_MUNMAP    12
#define SYS_MPROTECT  13
#define SYS_SIGRETURN 14

#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

intptr_t syscall(int num, uintptr_t p1, uintptr_t p2, uintptr_t p3);

/*
 * Syscall ABI (SYSCALL/IRETQ):
 *   RAX = syscall number
 *   RDI = arg1
 *   RSI = arg2          (RCX is taken by the CPU as the return-RIP)
 *   RDX = arg3          (R11 is taken by the CPU as the return-RFLAGS)
 * Return value in RAX.  The kernel returns through iretq, so every register
 * other than RAX/RCX/R11 comes back exactly as the caller left it.
 *
 * A note on the stack: the kernel's entry borrows one word of the *user* stack
 * ([rsp-8]) to carry the syscall number while it locates the kernel stack, and
 * restores rsp on return.  That word is inside the SysV red zone, so the libc
 * is built with -mno-red-zone (see meson.build) — otherwise a leaf function
 * compiling its locals at [rsp-8] would find them overwritten by every syscall.
 *
 * The libc test suite also compiles these units for the build machine, which is
 * x86_64 as well and has no kernel behind the instruction, so the stub
 * definitions below stand in.  `__CACT__` (set by the build, see meson.build)
 * is what tells the two apart — the architecture no longer does.
 */
#if defined(__x86_64__) && defined(__CACT__)
static inline intptr_t __syscall0(int num) {
    intptr_t ret;
    __asm__ volatile ("syscall"
        : "=a"(ret)
        : "a"(num)
        : "rcx", "r11", "memory");
    return ret;
}

static inline intptr_t __syscall1(int num, uintptr_t a1) {
    intptr_t ret;
    __asm__ volatile ("syscall"
        : "=a"(ret)
        : "a"(num), "D"(a1)
        : "rcx", "r11", "memory");
    return ret;
}

static inline intptr_t __syscall2(int num, uintptr_t a1, uintptr_t a2) {
    intptr_t ret;
    __asm__ volatile ("syscall"
        : "=a"(ret)
        : "a"(num), "D"(a1), "S"(a2)
        : "rcx", "r11", "memory");
    return ret;
}

static inline intptr_t __syscall3(int num, uintptr_t a1, uintptr_t a2, uintptr_t a3) {
    intptr_t ret;
    __asm__ volatile ("syscall"
        : "=a"(ret)
        : "a"(num), "D"(a1), "S"(a2), "d"(a3)
        : "rcx", "r11", "memory");
    return ret;
}
#else
static inline intptr_t __syscall0(int num) { (void)num; return -1; }
static inline intptr_t __syscall1(int num, uintptr_t a1) { (void)num; (void)a1; return -1; }
static inline intptr_t __syscall2(int num, uintptr_t a1, uintptr_t a2)
    { (void)num; (void)a1; (void)a2; return -1; }
static inline intptr_t __syscall3(int num, uintptr_t a1, uintptr_t a2, uintptr_t a3)
    { (void)num; (void)a1; (void)a2; (void)a3; return -1; }
#endif

#endif
