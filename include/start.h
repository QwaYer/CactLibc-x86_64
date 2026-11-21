#ifndef START_H
#define START_H

/* C linkage for the C++ consumers (libc++, LLVM, Mesa) that include this
 * header; a mangled reference would match no CactLibc export. */
#ifdef __cplusplus
extern "C" {
#endif

void _start();

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
