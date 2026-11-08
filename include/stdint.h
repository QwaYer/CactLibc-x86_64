#ifndef _STDINT_H
#define _STDINT_H

typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;
typedef signed long long int64_t;
typedef unsigned long long uint64_t;

typedef int64_t intmax_t;
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

#define UINTPTR_MAX 0xFFFFFFFFFFFFFFFFul

#define INT32_MIN  (-2147483647-1)
#define INT32_MAX  2147483647
#define UINT32_MAX 0xFFFFFFFFu

#endif
