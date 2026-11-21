#include "uchar.h"
#include "errno.h"

/* <uchar.h> for the C locale.  The execution character set is a single-byte
 * one, so a multibyte character is one code unit and no conversion is ever
 * stateful — the state object is only reset.  Bytes above 0x7f are not in the
 * C locale's character set, so they are EILSEQ rather than a silent
 * identity mapping. */

size_t mbrtoc16(char16_t *pc16, const char *s, size_t n, mbstate_t *ps) {
    unsigned char c;
    if (ps) ps->__state = 0;
    if (!s) return 0;
    if (n == 0) return (size_t)-2;
    c = (unsigned char)*s;
    if (c == 0) { if (pc16) *pc16 = 0; return 0; }
    if (c > 0x7f) { errno = EILSEQ; return (size_t)-1; }
    if (pc16) *pc16 = (char16_t)c;
    return 1;
}

size_t c16rtomb(char *s, char16_t c16, mbstate_t *ps) {
    if (ps) ps->__state = 0;
    if (!s) return 1;
    if (c16 > 0x7f) { errno = EILSEQ; return (size_t)-1; }
    *s = (char)c16;
    return 1;
}

size_t mbrtoc32(char32_t *pc32, const char *s, size_t n, mbstate_t *ps) {
    unsigned char c;
    if (ps) ps->__state = 0;
    if (!s) return 0;
    if (n == 0) return (size_t)-2;
    c = (unsigned char)*s;
    if (c == 0) { if (pc32) *pc32 = 0; return 0; }
    if (c > 0x7f) { errno = EILSEQ; return (size_t)-1; }
    if (pc32) *pc32 = (char32_t)c;
    return 1;
}

size_t c32rtomb(char *s, char32_t c32, mbstate_t *ps) {
    if (ps) ps->__state = 0;
    if (!s) return 1;
    if (c32 > 0x7f) { errno = EILSEQ; return (size_t)-1; }
    *s = (char)c32;
    return 1;
}
