/* thread_local host test: set_thread_area()'s request to the kernel.
 *
 * The registry half of thread_local.c (finding a thread's errno slot by stack
 * range) needs real threads and the real kernel, so it is not exercised here;
 * what is checked is that the %fs-base call carries the right procctl and base. */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

/* Layout-compatible with the libc's <sys/ldt.h> struct user_desc: entry_number,
 * then base_addr — a pointer-width word, so it sits at offset 8 on x86_64. */
struct user_desc {
    unsigned int  entry_number;
    unsigned long base_addr;
    unsigned int  limit;
    unsigned int  flags;
};

int  cact_set_thread_area(struct user_desc *u_info);
int *cact_tl_errno_location(void);

extern int           cact_host_ctl_calls;
extern unsigned long cact_host_ctl_cmd;
extern unsigned int  cact_host_ctl_arg0;
extern unsigned char cact_host_ctl_arg[24];
extern int           cact_host_ctl_ret;

#define CACT_PROCCTL_SET_TLS 0x3218

static int checks, fails;

/* The kernel reads a uint64_t base (CACT_PROCCTL_SET_TLS), so the test reads the
 * argument the same way rather than through the first-word echo. */
static uint64_t sent_base(void) {
    uint64_t b = 0;
    memcpy(&b, cact_host_ctl_arg, sizeof b);
    return b;
}

int main(void) {
    struct user_desc ud;
    memset(&ud, 0, sizeof ud);
    ud.entry_number = (unsigned int)-1;
    ud.base_addr    = 0x12345000;

    cact_host_ctl_calls = 0;
    cact_host_ctl_cmd = 0;
    cact_host_ctl_arg0 = 0;
    cact_host_ctl_ret = 0;

    int r = cact_set_thread_area(&ud);
    checks++;
    if (r != 0 || cact_host_ctl_calls != 1 ||
        cact_host_ctl_cmd != CACT_PROCCTL_SET_TLS ||
        sent_base() != 0x12345000ul) {
        fails++;
        printf("FAIL set: r=%d calls=%d cmd=%#lx base=%#lx\n",
               r, cact_host_ctl_calls, cact_host_ctl_cmd, (unsigned long)sent_base());
    }

    /* Clearing the base is a base of 0, not an error. */
    cact_host_ctl_calls = 0;
    cact_host_ctl_arg0 = 0xdeadbeef;
    ud.base_addr = 0;
    cact_set_thread_area(&ud);
    checks++;
    if (cact_host_ctl_calls != 1 || sent_base() != 0) {
        fails++;
        printf("FAIL clear: calls=%d base=%#lx\n", cact_host_ctl_calls,
               (unsigned long)sent_base());
    }

    /* NULL is rejected in the libc, without reaching the kernel. */
    cact_host_ctl_calls = 0;
    *cact_tl_errno_location() = 0;
    int r2 = cact_set_thread_area(0);
    int e2 = *cact_tl_errno_location();
    checks++;
    if (r2 != -1 || e2 != EFAULT || cact_host_ctl_calls != 0) {
        fails++;
        printf("FAIL null: r=%d errno=%d calls=%d\n", r2, e2, cact_host_ctl_calls);
    }

    /* A kernel failure maps to -1. */
    cact_host_ctl_ret = -1;
    int r3 = cact_set_thread_area(&ud);
    checks++;
    if (r3 != -1) { fails++; printf("FAIL err propagation: r=%d\n", r3); }

    printf("thread_local: %d checks, %d failed\n", checks, fails);
    return fails ? 1 : 0;
}
