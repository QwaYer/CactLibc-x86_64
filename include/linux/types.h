#ifndef _LINUX_TYPES_H
#define _LINUX_TYPES_H

/* C linkage for the C++ consumers (libc++, LLVM, Mesa) that include this
 * header; a mangled reference would match no CactLibc export. */
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/*
 * <linux/types.h> — the fixed-width names the kernel uapi headers use.
 *
 * On a glibc system this comes from linux-libc-dev; CactOS ships the uapi
 * headers a program ports (Mesa's include/drm-uapi/*, libdrm) need, and they
 * spell their fields __u32/__u64/__le32 and so on.  Only the types are here —
 * the rest of <linux/types.h> (the kernel's internal structures) is not part of
 * any uapi a program should include.
 */
typedef uint8_t  __u8;
typedef uint16_t __u16;
typedef uint32_t __u32;
typedef uint64_t __u64;

typedef int8_t   __s8;
typedef int16_t  __s16;
typedef int32_t  __s32;
typedef int64_t  __s64;

typedef __u16 __le16;
typedef __u32 __le32;
typedef __u64 __le64;
typedef __u16 __be16;
typedef __u32 __be32;
typedef __u64 __be64;

typedef __u16 __sum16;
typedef __u32 __wsum;

typedef unsigned long __kernel_ulong_t;
typedef long          __kernel_long_t;
typedef unsigned int  __kernel_uint_t;
typedef int           __kernel_int_t;

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* _LINUX_TYPES_H */
