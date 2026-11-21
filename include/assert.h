#ifndef _ASSERT_H
#define _ASSERT_H

/* C linkage for the C++ consumers (libc++, LLVM, Mesa) that include this
 * header; a mangled reference would match no CactLibc export. */
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#ifndef NDEBUG
#define assert(x) do { if (!(x)) { fprintf(stderr, "Assertion failed: %s, file %s, line %d\n", #x, __FILE__, __LINE__); abort(); } } while(0)
#else
#define assert(x) ((void)0)
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
