#ifndef _STRINGS_H
#define _STRINGS_H

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

#endif /* _STRINGS_H */
