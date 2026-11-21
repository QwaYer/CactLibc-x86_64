#ifndef _SYS_PARAM_H
#define _SYS_PARAM_H

/* C linkage for the C++ consumers (libc++, LLVM, Mesa) that include this
 * header; a mangled reference would match no CactLibc export. */
#ifdef __cplusplus
extern "C" {
#endif

#include <limits.h>     /* PATH_MAX */
#include <stdint.h>

/* BSD's catch-all system header, as glibc provides it: the path-size names, the
 * MIN/MAX macros and the small bit helpers.  Mesa's loader includes it for
 * PATH_MAX/MAXPATHLEN. */
#ifndef MAXPATHLEN
#define MAXPATHLEN PATH_MAX
#endif

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN3(a, b, c) MIN(MIN(a, b), c)
#define MAX3(a, b, c) MAX(MAX(a, b), c)

#define NBBY 8                              /* bits per byte */
#define howmany(x, y) (((x) + ((y) - 1)) / (y))
#define roundup(x, y) ((((x) + ((y) - 1)) / (y)) * (y))
#define powerof2(x) ((((x) - 1) & (x)) == 0)

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* _SYS_PARAM_H */
