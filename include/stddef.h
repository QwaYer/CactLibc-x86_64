#ifndef _STDDEF_H
#define _STDDEF_H

/* C linkage for the C++ consumers (libc++, LLVM, Mesa) that include this
 * header; a mangled reference would match no CactLibc export. */
#ifdef __cplusplus
extern "C" {
#endif

typedef __SIZE_TYPE__    size_t;
typedef __PTRDIFF_TYPE__ ptrdiff_t;

/* C11 7.19: <stddef.h> also has to provide wchar_t (a keyword in C++, a
 * typedef here) and max_align_t.  max_align_t is the widest fundamental
 * alignment, which on x86_64 is 16 — long double's.  A C++ standard library
 * header reads it (libc++'s std::pmr/allocator defaults are alignof(max_align_t)). */
#ifndef __cplusplus
typedef __WCHAR_TYPE__ wchar_t;
#endif

typedef struct {
    long long   __ll;
    long double __ld;
} max_align_t;

/* NULL must be a *null pointer constant* in C++: `((void*)0)` is not one, so
 * `char* p = NULL` would not compile.  GCC's and clang's own <stddef.h> use
 * the __null extension for C++ and the cast for C; same here. */
#ifdef __cplusplus
#define NULL __null
#else
#define NULL ((void*)0)
#endif

#define offsetof(type, member) __builtin_offsetof(type, member)

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
