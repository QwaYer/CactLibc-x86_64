#ifndef _LIMITS_H
#define _LIMITS_H

/* C linkage for the C++ consumers (libc++, LLVM, Mesa) that include this
 * header; a mangled reference would match no CactLibc export. */
#ifdef __cplusplus
extern "C" {
#endif

/* Implementation-defined limits for CactOS/x86_64.
 *
 * The integer limits are the LP64 ones (long is 64-bit, int 32-bit); the path
 * and name limits are the VFS's, not the C library's: a VFS node carries a
 * 128-byte name (vfs_node_t.name), so NAME_MAX is 127, and PATH_MAX is the
 * Linux-compatible 4096 the path helpers size their buffers for. */

#define CHAR_BIT   8
#define SCHAR_MIN  (-128)
#define SCHAR_MAX  127
#define UCHAR_MAX  255
#define CHAR_MIN   SCHAR_MIN
#define CHAR_MAX   SCHAR_MAX

#define SHRT_MIN   (-32768)
#define SHRT_MAX   32767
#define USHRT_MAX  65535

#define INT_MIN    (-2147483647 - 1)
#define INT_MAX    2147483647
#define UINT_MAX   4294967295U

#define LONG_MIN   (-9223372036854775807L - 1L)
#define LONG_MAX   9223372036854775807L
#define ULONG_MAX  18446744073709551615UL

#define LLONG_MIN  (-9223372036854775807LL - 1LL)
#define LLONG_MAX  9223372036854775807LL
#define ULLONG_MAX 18446744073709551615ULL

#define MB_LEN_MAX 16

#define WORD_BIT  32
#define LONG_BIT  64

/* POSIX <limits.h> additions. */
#define PATH_MAX      4096
#define NAME_MAX      127
#define PIPE_BUF      4096
#define OPEN_MAX      1024
#define ARG_MAX       131072
#define LINE_MAX      2048
#define SSIZE_MAX     LONG_MAX
#define NZERO         20
#define HOST_NAME_MAX 64
#define LOGIN_NAME_MAX 256
#define TTY_NAME_MAX  32
#define SYMLOOP_MAX   40
#define NGROUPS_MAX   32
#define NL_ARGMAX     9

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* _LIMITS_H */
