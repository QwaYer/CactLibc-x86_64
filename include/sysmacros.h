#ifndef _SYS_SYSMACROS_H
#define _SYS_SYSMACROS_H

/* C linkage for the C++ consumers (libc++, LLVM, Mesa) that include this
 * header; a mangled reference would match no CactLibc export. */
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define major(dev)   ((unsigned int)(((dev) >> 8) & 0xfff))
#define minor(dev)   ((unsigned int)(((dev) & 0xff) | (((dev) >> 12) & 0xfff00)))
#define makedev(maj, min) \
    ((unsigned int)((((maj) & 0xfff) << 8) | ((min) & 0xff) | (((min) & 0xfff00) << 12)))

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
