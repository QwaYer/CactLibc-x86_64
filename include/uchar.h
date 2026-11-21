#ifndef _UCHAR_H
#define _UCHAR_H

/* C linkage for the C++ consumers (libc++, LLVM, Mesa) that include this
 * header; a mangled reference would match no CactLibc export. */
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>   /* size_t */

/* C11 7.28 <uchar.h>.
 *
 * CactOS has exactly one encoding — the C locale's — so every conversion
 * between the execution character set and a 16- or 32-bit code unit is the
 * identity on the byte values and never becomes stateful.  The header exists
 * because <mbstate_t> has no other home on this target: C++'s std::mbstate_t
 * (libc++'s <__mbstate_t.h>) is taken from <wchar.h> or <uchar.h>, and CactOS
 * ships neither a wchar.h nor a wide-character string library. */

typedef struct {
    unsigned int __state;
} mbstate_t;

/* char16_t and char32_t are C11 keywords only in C++; in C they are typedefs
 * from <uchar.h>.  libc++ does `#include_next <uchar.h>` from C++, so the
 * typedefs must not be visible there. */
#ifndef __cplusplus
typedef unsigned short char16_t;   /* uint_least16_t */
typedef unsigned int   char32_t;   /* uint_least32_t */

size_t mbrtoc16(char16_t *pc16, const char *s, size_t n, mbstate_t *ps);
size_t c16rtomb(char *s, char16_t c16, mbstate_t *ps);
size_t mbrtoc32(char32_t *pc32, const char *s, size_t n, mbstate_t *ps);
size_t c32rtomb(char *s, char32_t c32, mbstate_t *ps);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* _UCHAR_H */
