#ifndef _SYS_LDT_H
#define _SYS_LDT_H

/* C linkage for the C++ consumers (libc++, LLVM, Mesa) that include this
 * header; a mangled reference would match no CactLibc export. */
#ifdef __cplusplus
extern "C" {
#endif

/* CactOS keeps no LDT: the kernel programs IA32_FS_BASE for the task directly
 * (see CACT_PROCCTL_SET_TLS).  set_thread_area() installs `base_addr` as the
 * task's `%fs` base, so the function is source-compatible with the Linux i386
 * one while the rest of the structure has nothing to describe and is ignored
 * (limit, the segment flags and the chosen entry_number all have no effect;
 * (unsigned)-1 for entry_number — "pick one" — is equally a no-op).
 *
 * `base_addr` must be a user address, 16-byte aligned (the x86_64 ABI's
 * requirement for the thread pointer) or zero; anything else is EINVAL.  The
 * static TLS layout rounds its block to 16, so what __cact_tls_start() installs
 * always satisfies it. */

struct user_desc {
    unsigned int  entry_number;
    unsigned long base_addr;
    unsigned int  limit;
    unsigned int  seg_32bit:1;
    unsigned int  contents:2;
    unsigned int  read_exec_only:1;
    unsigned int  limit_in_pages:1;
    unsigned int  seg_not_present:1;
    unsigned int  useable:1;
};

/* Set the calling task's %fs base to u_info->base_addr.  0 clears it. */
int set_thread_area(struct user_desc *u_info);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
