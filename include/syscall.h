#ifndef _SYSCALL_H
#define _SYSCALL_H

/* C linkage for the C++ consumers (libc++, LLVM, Mesa) that include this
 * header; a mangled reference would match no CactLibc export. */
#ifdef __cplusplus
extern "C" {
#endif

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

/* Pseudo-number answered in the library, not by the kernel: CactOS's syscall
 * ABI has no gettid (the tid is a /proc/self datum here — see CactLibc's
 * gettid()), but a ported program reaches for the Linux `syscall(SYS_gettid)`
 * idiom (Mesa's thread-priority path does).  src/syscall.c returns gettid() for
 * it; a raw syscall instruction with this number would get -1 from the kernel,
 * so nothing else should use the value. */
#define SYS_gettid    15

#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

/* Variadic, like POSIX's syscall(2): the kernel takes three words, but a C++
 * caller passes a pointer for one of them and only `...` accepts that without
 * a cast.  The three words are read from the argument list; see src/syscall.c. */
intptr_t syscall(int num, ...);

/*
 * Syscall ABI (SYSCALL/IRETQ):
 *   RAX = syscall number
 *   RDI = arg1
 *   RSI = arg2          (RCX is taken by the CPU as the return-RIP)
 *   RDX = arg3          (R11 is taken by the CPU as the return-RFLAGS)
 * Return value in RAX.  The kernel returns through iretq, so every register
 * other than RAX/RCX/R11 comes back exactly as the caller left it.
 *
 * A note on the stack: `syscall` does not switch stacks, but the kernel's entry
 * goes straight from the user rsp to this CPU's kernel stack (one %gs-relative
 * load, with the user rsp kept in r10), so it writes nothing below the user rsp
 * — the SysV red zone survives a syscall and no -mno-red-zone is needed.
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

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
