#ifndef VERSION_H
#define VERSION_H

/* C linkage for the C++ consumers (libc++, LLVM, Mesa) that include this
 * header; a mangled reference would match no CactLibc export. */
#ifdef __cplusplus
extern "C" {
#endif

#define CACTLIB_VERSION_MAJOR 2
#define CACTLIB_VERSION_MINOR 0
#define CACTLIB_VERSION_PATCH 0

#define CACTLIB_VERSION "2.0.0"

#define CACTLIB_VERSION_NUMBER \
    ((CACTLIB_VERSION_MAJOR << 16) | (CACTLIB_VERSION_MINOR << 8) | CACTLIB_VERSION_PATCH)

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif