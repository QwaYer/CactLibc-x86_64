#ifndef _SYS_TYPES_H
#define _SYS_TYPES_H

/* C linkage for the C++ consumers (libc++, LLVM, Mesa) that include this
 * header; a mangled reference would match no CactLibc export. */
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include "unistd.h"

typedef unsigned long ulong;
typedef unsigned int  uint;

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
