#ifndef _STRINGS_H
#define _STRINGS_H

/* C linkage for the C++ consumers (libc++, LLVM, Mesa) that include this
 * header; a mangled reference would match no CactLibc export. */
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/* POSIX <strings.h> — the BSD-derived string helpers.  The case-insensitive
 * compares are also declared in <string.h>, where they historically lived. */

int   strcasecmp(const char *s1, const char *s2);
int   strncasecmp(const char *s1, const char *s2, size_t n);

int   ffs(int i);                                    /* 1-based first set bit, 0 if none */
char *index(const char *s, int c);                   /* strchr under its BSD name */
char *rindex(const char *s, int c);                  /* strrchr under its BSD name */
void  bzero(void *s, size_t n);                      /* memset(s, 0, n) */
void  bcopy(const void *src, void *dst, size_t n);   /* memmove(dst, src, n) */
int   bcmp(const void *s1, const void *s2, size_t n); /* memcmp */

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* _STRINGS_H */
