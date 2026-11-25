#ifndef _ASSERT_H
#define _ASSERT_H

/* C linkage for the C++ consumers (libc++, LLVM, Mesa) that include this
 * header; a mangled reference would match no CactLibc export. */
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>   /* abort(), which the failing assert() below calls */

#ifndef NDEBUG
#define assert(x) do { if (!(x)) { fprintf(stderr, "Assertion failed: %s, file %s, line %d\n", #x, __FILE__, __LINE__); abort(); } } while(0)
#else
#define assert(x) ((void)0)
#endif

/* C11's <assert.h> supplies static_assert for C (in C23 it is also a keyword).
 * Mesa's ralloc.c uses it in C translation units. */
#if !defined(__cplusplus) && defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#ifndef static_assert
#define static_assert _Static_assert
#endif
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
