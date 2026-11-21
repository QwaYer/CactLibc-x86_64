#ifndef LIBGEN_H
#define LIBGEN_H

/* C linkage for the C++ consumers (libc++, LLVM, Mesa) that include this
 * header; a mangled reference would match no CactLibc export. */
#ifdef __cplusplus
extern "C" {
#endif

char *dirname(char *path);
char *basename(char *path);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
