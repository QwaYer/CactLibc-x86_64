#ifndef _ALLOCA_H
#define _ALLOCA_H

/* alloca(3): memory in the caller's stack frame.  clang lowers the builtin to a
 * stack adjustment, so no library support is needed; the wrapper exists because
 * a program includes <alloca.h> (or gets it from <stdlib.h> under _GNU_SOURCE)
 * and calls alloca() by name. */
#ifdef __cplusplus
extern "C" {
#endif

#define alloca(size) __builtin_alloca(size)

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* _ALLOCA_H */
