#ifndef _SYS_RANDOM_H
#define _SYS_RANDOM_H

/* C linkage for the C++ consumers (libc++, LLVM, Mesa) that include this
 * header; a mangled reference would match no CactLibc export. */
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

#define GRND_NONBLOCK 0x0001
#define GRND_RANDOM   0x0002

int getrandom(void *buf, size_t buflen, unsigned int flags);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
