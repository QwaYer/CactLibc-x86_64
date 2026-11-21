#ifndef _STDINT_H
#define _STDINT_H

/* C linkage for the C++ consumers (libc++, LLVM, Mesa) that include this
 * header; a mangled reference would match no CactLibc export. */
#ifdef __cplusplus
extern "C" {
#endif

/* C99 7.18 — integer types and their limits.
 *
 * The exact-width types are the ones most code uses; the least/fast families
 * and the limit macros are here because a C++ standard library header takes
 * them from <cstdint> (libc++'s <cstdint> is a list of `using ::x` lines, and
 * a type it names but the C library does not is a hard error there). */

typedef signed char        int8_t;
typedef unsigned char      uint8_t;
typedef signed short       int16_t;
typedef unsigned short     uint16_t;
typedef signed int         int32_t;
typedef unsigned int       uint32_t;
typedef signed long long   int64_t;
typedef unsigned long long uint64_t;

/* "least": the smallest type with at least N bits — the exact-width ones.
 * "fast": the natural width — on x86_64 that is 64 bits for everything a byte
 * register cannot hold, which is what glibc picks too. */
typedef int8_t   int_least8_t;
typedef uint8_t  uint_least8_t;
typedef int16_t  int_least16_t;
typedef uint16_t uint_least16_t;
typedef int32_t  int_least32_t;
typedef uint32_t uint_least32_t;
typedef int64_t  int_least64_t;
typedef uint64_t uint_least64_t;

typedef signed char        int_fast8_t;
typedef unsigned char      uint_fast8_t;
typedef long long          int_fast16_t;
typedef unsigned long long uint_fast16_t;
typedef long long          int_fast32_t;
typedef unsigned long long uint_fast32_t;
typedef long long          int_fast64_t;
typedef unsigned long long uint_fast64_t;

typedef int64_t  intmax_t;
typedef uint64_t uintmax_t;

/* x86_64 (LP64): a pointer and a `long` are 64 bits.  `long`/`unsigned long`
 * rather than int64_t/uint64_t on purpose — the host test harness compiles one
 * side of the shapes shared with the loaders on a 64-bit machine, and the SysV
 * ABI's own intptr_t is `long`; keeping the builtin type means both sides agree.
 * Everything that carries an address (brk, mmap, the thread registry, the ABI
 * structs) hangs off these two typedefs, so this is the single place where the
 * pointer width is decided. */
typedef long          intptr_t;
typedef unsigned long uintptr_t;

/* 7.18.2.1 limits of exact-width types */
#define INT8_MIN    (-128)
#define INT8_MAX    127
#define UINT8_MAX   255u
#define INT16_MIN   (-32767-1)
#define INT16_MAX   32767
#define UINT16_MAX  65535u
#define INT32_MIN   (-2147483647-1)
#define INT32_MAX   2147483647
#define UINT32_MAX  0xFFFFFFFFu
#define INT64_MIN   (-9223372036854775807LL-1)
#define INT64_MAX   9223372036854775807LL
#define UINT64_MAX  0xFFFFFFFFFFFFFFFFull

/* 7.18.2.2 limits of the minimum-width, fastest and other types */
#define INT_LEAST8_MIN   INT8_MIN
#define INT_LEAST8_MAX   INT8_MAX
#define UINT_LEAST8_MAX  UINT8_MAX
#define INT_LEAST16_MIN  INT16_MIN
#define INT_LEAST16_MAX  INT16_MAX
#define UINT_LEAST16_MAX UINT16_MAX
#define INT_LEAST32_MIN  INT32_MIN
#define INT_LEAST32_MAX  INT32_MAX
#define UINT_LEAST32_MAX UINT32_MAX
#define INT_LEAST64_MIN  INT64_MIN
#define INT_LEAST64_MAX  INT64_MAX
#define UINT_LEAST64_MAX UINT64_MAX

#define INT_FAST8_MIN    INT8_MIN
#define INT_FAST8_MAX    INT8_MAX
#define UINT_FAST8_MAX   UINT8_MAX
#define INT_FAST16_MIN   INT64_MIN
#define INT_FAST16_MAX   INT64_MAX
#define UINT_FAST16_MAX  UINT64_MAX
#define INT_FAST32_MIN   INT64_MIN
#define INT_FAST32_MAX   INT64_MAX
#define UINT_FAST32_MAX  UINT64_MAX
#define INT_FAST64_MIN   INT64_MIN
#define INT_FAST64_MAX   INT64_MAX
#define UINT_FAST64_MAX  UINT64_MAX

#define INTPTR_MIN   (-9223372036854775807L-1)
#define INTPTR_MAX   9223372036854775807L
#define UINTPTR_MAX  0xFFFFFFFFFFFFFFFFul
#define INTMAX_MIN   INT64_MIN
#define INTMAX_MAX   INT64_MAX
#define UINTMAX_MAX  UINT64_MAX

#define PTRDIFF_MIN  (-9223372036854775807L-1)
#define PTRDIFF_MAX  9223372036854775807L
#define SIZE_MAX     UINTPTR_MAX

/* wchar_t and wint_t are 32-bit signed on x86_64.  The limits belong to the
 * types, so a header that includes <stdint.h> must find them (C99 7.18.3);
 * <wchar.h> defines the same WCHAR pair, so guard against it. */
#ifndef WCHAR_MIN
#define WCHAR_MIN    (-2147483647-1)
#define WCHAR_MAX    2147483647
#endif
#ifndef WINT_MIN
#define WINT_MIN     (-2147483647-1)
#define WINT_MAX     2147483647
#endif

/* 7.18.4 greatest-width integer constants */
#define INT8_C(c)    c
#define INT16_C(c)   c
#define INT32_C(c)   c
#define INT64_C(c)   c##LL
#define UINT8_C(c)   c
#define UINT16_C(c)  c
#define UINT32_C(c)  c##u
#define UINT64_C(c)  c##ULL
#define INTMAX_C(c)  c##LL
#define UINTMAX_C(c) c##ULL

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
