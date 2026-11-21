#include "wchar.h"
#include "stdlib.h"
#include "string.h"
#include "errno.h"

/* <wchar.h> for the C locale (see the header for the conventions).
 *
 * One multibyte code unit is one byte, so the wide *string* and *memory*
 * operations are the byte ones widened, and the conversion functions are the
 * <uchar.h> ones with the wchar_t spelling.  The two places that need real work
 * are the handful that call into the byte-only formatter and the strto* family:
 * both narrow their wide input (the C locale is byte-per-code-unit) and widen
 * the result. */

/* ── wide memory ──────────────────────────────────────────────────────────── */

wchar_t *wmemcpy(wchar_t *dst, const wchar_t *src, size_t n) {
    return (wchar_t *)memcpy(dst, src, n * sizeof(wchar_t));
}

wchar_t *wmemmove(wchar_t *dst, const wchar_t *src, size_t n) {
    return (wchar_t *)memmove(dst, src, n * sizeof(wchar_t));
}

int wmemcmp(const wchar_t *a, const wchar_t *b, size_t n) {
    for (size_t i = 0; i < n; i++) {
        if (a[i] != b[i]) return a[i] < b[i] ? -1 : 1;
    }
    return 0;
}

wchar_t *wmemchr(const wchar_t *s, wchar_t c, size_t n) {
    for (size_t i = 0; i < n; i++)
        if (s[i] == c) return (wchar_t *)&s[i];
    return NULL;
}

wchar_t *wmemset(wchar_t *s, wchar_t c, size_t n) {
    for (size_t i = 0; i < n; i++) s[i] = c;
    return s;
}

/* ── wide strings ─────────────────────────────────────────────────────────── */

size_t wcslen(const wchar_t *s) {
    const wchar_t *p = s;
    while (*p) p++;
    return (size_t)(p - s);
}

size_t wcsnlen(const wchar_t *s, size_t maxlen) {
    size_t i = 0;
    while (i < maxlen && s[i]) i++;
    return i;
}

wchar_t *wcscpy(wchar_t *dst, const wchar_t *src) {
    wchar_t *d = dst;
    while ((*d++ = *src++))
        ;
    return dst;
}

wchar_t *wcsncpy(wchar_t *dst, const wchar_t *src, size_t n) {
    size_t i = 0;
    for (; i < n && src[i]; i++) dst[i] = src[i];
    for (; i < n; i++) dst[i] = L'\0';
    return dst;
}

wchar_t *wcscat(wchar_t *dst, const wchar_t *src) {
    wchar_t *d = dst + wcslen(dst);
    while ((*d++ = *src++))
        ;
    return dst;
}

wchar_t *wcsncat(wchar_t *dst, const wchar_t *src, size_t n) {
    wchar_t *d = dst + wcslen(dst);
    size_t i = 0;
    for (; i < n && src[i]; i++) d[i] = src[i];
    d[i] = L'\0';
    return dst;
}

int wcscmp(const wchar_t *a, const wchar_t *b) {
    while (*a && *a == *b) { a++; b++; }
    return (*a > *b) - (*a < *b);
}

int wcsncmp(const wchar_t *a, const wchar_t *b, size_t n) {
    for (size_t i = 0; i < n; i++) {
        if (a[i] != b[i]) return (a[i] > b[i]) - (a[i] < b[i]);
        if (a[i] == 0) break;
    }
    return 0;
}

/* One locale: collation is code-unit order. */
int wcscoll(const wchar_t *a, const wchar_t *b) { return wcscmp(a, b); }

size_t wcsxfrm(wchar_t *dst, const wchar_t *src, size_t n) {
    size_t len = wcslen(src);
    if (n) {
        size_t c = len < n - 1 ? len : n - 1;
        wmemcpy(dst, src, c);
        dst[c] = L'\0';
    }
    return len;
}

wchar_t *wcschr(const wchar_t *s, wchar_t c) {
    for (;; s++) {
        if (*s == c) return (wchar_t *)s;
        if (!*s) return NULL;
    }
}

wchar_t *wcsrchr(const wchar_t *s, wchar_t c) {
    const wchar_t *last = NULL;
    for (;; s++) {
        if (*s == c) last = s;
        if (!*s) return (wchar_t *)last;
    }
}

wchar_t *wcsstr(const wchar_t *haystack, const wchar_t *needle) {
    if (!*needle) return (wchar_t *)haystack;
    for (; *haystack; haystack++) {
        const wchar_t *h = haystack, *n = needle;
        while (*h && *h == *n) { h++; n++; }
        if (!*n) return (wchar_t *)haystack;
    }
    return NULL;
}

wchar_t *wcspbrk(const wchar_t *s, const wchar_t *accept) {
    for (; *s; s++)
        if (wcschr(accept, *s)) return (wchar_t *)s;
    return NULL;
}

size_t wcsspn(const wchar_t *s, const wchar_t *accept) {
    const wchar_t *p = s;
    while (*p && wcschr(accept, *p)) p++;
    return (size_t)(p - s);
}

size_t wcscspn(const wchar_t *s, const wchar_t *reject) {
    const wchar_t *p = s;
    while (*p && !wcschr(reject, *p)) p++;
    return (size_t)(p - s);
}

wchar_t *wcstok(wchar_t *s, const wchar_t *delim, wchar_t **saveptr) {
    wchar_t *p = s ? s : (saveptr ? *saveptr : NULL);
    if (!p) return NULL;
    p += wcsspn(p, delim);            /* skip leading delimiters */
    if (!*p) { if (saveptr) *saveptr = p; return NULL; }
    wchar_t *tok = p;
    p += wcscspn(p, delim);           /* scan to the next delimiter */
    if (*p) { *p = L'\0'; if (saveptr) *saveptr = p + 1; }
    else if (saveptr) *saveptr = NULL;
    return tok;
}

wchar_t *wcsdup(const wchar_t *s) {
    size_t n = wcslen(s) + 1;
    wchar_t *d = malloc(n * sizeof(wchar_t));
    if (d) wmemcpy(d, s, n);
    return d;
}

/* ── conversions between the multibyte and wide encodings ──────────────────── */

wint_t btowc(int c) {
    if (c == EOF || (unsigned int)c > 0x7f) return WEOF;
    return (wint_t)c;
}

int wctob(wint_t c) {
    if (c == WEOF || c > 0x7f) return EOF;
    return (int)c;
}

int mbsinit(const mbstate_t *ps) {
    return !ps || ps->__state == 0;
}

size_t mbrtowc(wchar_t *pwc, const char *s, size_t n, mbstate_t *ps) {
    unsigned char c;
    if (ps) ps->__state = 0;
    if (!s) return 0;
    if (n == 0) return (size_t)-2;
    c = (unsigned char)*s;
    if (c == 0) { if (pwc) *pwc = L'\0'; return 0; }
    if (c > 0x7f) { errno = EILSEQ; return (size_t)-1; }
    if (pwc) *pwc = (wchar_t)c;
    return 1;
}

size_t mbrlen(const char *s, size_t n, mbstate_t *ps) {
    if (ps) ps->__state = 0;
    if (!s) return 0;
    if (n == 0) return (size_t)-2;
    if ((unsigned char)*s == 0) return 0;
    if ((unsigned char)*s > 0x7f) { errno = EILSEQ; return (size_t)-1; }
    return 1;
}

size_t wcrtomb(char *s, wchar_t wc, mbstate_t *ps) {
    if (ps) ps->__state = 0;
    if (!s) return 1;                 /* reset to the initial state */
    if ((unsigned int)wc > 0x7f) { errno = EILSEQ; return (size_t)-1; }
    *s = (char)wc;
    return 1;
}

size_t mbsrtowcs(wchar_t *dst, const char **src, size_t len, mbstate_t *ps) {
    if (ps) ps->__state = 0;
    const char *s = *src;
    size_t i = 0;
    while (*s) {
        if ((unsigned char)*s > 0x7f) { errno = EILSEQ; return (size_t)-1; }
        if (dst) {
            if (i >= len) { *src = s; return i; }
            dst[i] = (wchar_t)(unsigned char)*s;
        }
        i++; s++;
    }
    if (dst) {
        if (i < len) dst[i] = L'\0';
        else { *src = s; return i; }
    }
    *src = NULL;
    return i;
}

size_t wcsrtombs(char *dst, const wchar_t **src, size_t len, mbstate_t *ps) {
    if (ps) ps->__state = 0;
    const wchar_t *s = *src;
    size_t i = 0;
    while (*s) {
        if ((unsigned int)*s > 0x7f) { errno = EILSEQ; return (size_t)-1; }
        if (dst) {
            if (i >= len) { *src = s; return i; }
            dst[i] = (char)*s;
        }
        i++; s++;
    }
    if (dst) {
        if (i < len) dst[i] = '\0';
        else { *src = s; return i; }
    }
    *src = NULL;
    return i;
}

/* The POSIX bounded forms: as above, but consuming at most nms bytes of the
 * source (mbsnrtowcs) or at most nwc wide characters (wcsnrtombs).  They obey
 * the same *src convention. */
size_t mbsnrtowcs(wchar_t *dst, const char **src, size_t nms, size_t len,
                  mbstate_t *ps) {
    if (ps) ps->__state = 0;
    const char *s = *src;
    const char *end = s + nms;
    size_t i = 0;
    while (s < end && *s) {
        if ((unsigned char)*s > 0x7f) { errno = EILSEQ; return (size_t)-1; }
        if (dst) {
            if (i >= len) { *src = s; return i; }
            dst[i] = (wchar_t)(unsigned char)*s;
        }
        i++; s++;
    }
    if (s >= end) { *src = s; return i; }   /* stopped by the byte limit */
    if (dst) {
        if (i < len) dst[i] = L'\0';
        else { *src = s; return i; }
    }
    *src = NULL;
    return i;
}

size_t wcsnrtombs(char *dst, const wchar_t **src, size_t nwc, size_t len,
                  mbstate_t *ps) {
    if (ps) ps->__state = 0;
    const wchar_t *s = *src;
    const wchar_t *end = s + nwc;
    size_t i = 0;
    while (s < end && *s) {
        if ((unsigned int)*s > 0x7f) { errno = EILSEQ; return (size_t)-1; }
        if (dst) {
            if (i >= len) { *src = s; return i; }
            dst[i] = (char)*s;
        }
        i++; s++;
    }
    if (s >= end) { *src = s; return i; }   /* stopped by the wide limit */
    if (dst) {
        if (i < len) dst[i] = '\0';
        else { *src = s; return i; }
    }
    *src = NULL;
    return i;
}

/* The <stdlib.h> functions of the wide set. */
int mblen(const char *s, size_t n) {
    if (!s) return 0;
    if (n == 0) return -1;
    if ((unsigned char)*s == 0) return 0;
    if ((unsigned char)*s > 0x7f) { errno = EILSEQ; return -1; }
    return 1;
}

/* C89 mbtowc()/wctomb(): the stateless twins of mbrtowc()/wcrtomb(). */
int mbtowc(wchar_t *pwc, const char *s, size_t n) {
    if (!s) return 0;                 /* reset */
    if (n == 0) return -1;
    if ((unsigned char)*s == 0) { if (pwc) *pwc = L'\0'; return 0; }
    if ((unsigned char)*s > 0x7f) { errno = EILSEQ; return -1; }
    if (pwc) *pwc = (wchar_t)(unsigned char)*s;
    return 1;
}

int wctomb(char *s, wchar_t wc) {
    if (!s) return 0;                 /* reset */
    if ((unsigned int)wc > 0x7f) { errno = EILSEQ; return -1; }
    *s = (char)wc;
    return 1;
}

size_t mbstowcs(wchar_t *dst, const char *src, size_t n) {
    size_t i = 0;
    while (src[i]) {
        if ((unsigned char)src[i] > 0x7f) { errno = EILSEQ; return (size_t)-1; }
        if (dst) {
            if (i >= n) break;
            dst[i] = (wchar_t)(unsigned char)src[i];
        }
        i++;
    }
    if (dst && i < n) dst[i] = L'\0';
    return i;
}

size_t wcstombs(char *dst, const wchar_t *src, size_t n) {
    size_t i = 0;
    while (src[i]) {
        if ((unsigned int)src[i] > 0x7f) { errno = EILSEQ; return (size_t)-1; }
        if (dst) {
            if (i >= n) break;
            dst[i] = (char)src[i];
        }
        i++;
    }
    if (dst && i < n) dst[i] = '\0';
    return i;
}

/* ── numeric conversions ──────────────────────────────────────────────────────
 * The C locale narrows one code unit to one byte, so the strto* family parses
 * the narrowed copy and the wide end pointer is recovered from the narrow one. */
static char *narrow_dup(const wchar_t *ws) {
    size_t n = wcslen(ws);
    char *ns = malloc(n + 1);
    if (!ns) return NULL;
    for (size_t i = 0; i <= n; i++) ns[i] = (char)ws[i];
    return ns;
}

static wchar_t *un_narrow(const wchar_t *base, const char *nbase, char *end) {
    return (wchar_t *)base + (size_t)(end - nbase);
}

long wcstol(const wchar_t *nptr, wchar_t **endptr, int base) {
    char *ns = narrow_dup(nptr);
    if (!ns) { if (endptr) *endptr = (wchar_t *)nptr; return 0; }
    char *e;
    long v = strtol(ns, &e, base);
    if (endptr) *endptr = un_narrow(nptr, ns, e);
    free(ns);
    return v;
}

unsigned long wcstoul(const wchar_t *nptr, wchar_t **endptr, int base) {
    char *ns = narrow_dup(nptr);
    if (!ns) { if (endptr) *endptr = (wchar_t *)nptr; return 0; }
    char *e;
    unsigned long v = strtoul(ns, &e, base);
    if (endptr) *endptr = un_narrow(nptr, ns, e);
    free(ns);
    return v;
}

long long wcstoll(const wchar_t *nptr, wchar_t **endptr, int base) {
    char *ns = narrow_dup(nptr);
    if (!ns) { if (endptr) *endptr = (wchar_t *)nptr; return 0; }
    char *e;
    long long v = strtoll(ns, &e, base);
    if (endptr) *endptr = un_narrow(nptr, ns, e);
    free(ns);
    return v;
}

unsigned long long wcstoull(const wchar_t *nptr, wchar_t **endptr, int base) {
    char *ns = narrow_dup(nptr);
    if (!ns) { if (endptr) *endptr = (wchar_t *)nptr; return 0; }
    char *e;
    unsigned long long v = strtoull(ns, &e, base);
    if (endptr) *endptr = un_narrow(nptr, ns, e);
    free(ns);
    return v;
}

double wcstod(const wchar_t *nptr, wchar_t **endptr) {
    char *ns = narrow_dup(nptr);
    if (!ns) { if (endptr) *endptr = (wchar_t *)nptr; return 0.0; }
    char *e;
    double v = strtod(ns, &e);
    if (endptr) *endptr = un_narrow(nptr, ns, e);
    free(ns);
    return v;
}

float wcstof(const wchar_t *nptr, wchar_t **endptr) {
    return (float)wcstod(nptr, endptr);
}

long double wcstold(const wchar_t *nptr, wchar_t **endptr) {
    char *ns = narrow_dup(nptr);
    if (!ns) { if (endptr) *endptr = (wchar_t *)nptr; return 0.0L; }
    char *e;
    long double v = strtold(ns, &e);
    if (endptr) *endptr = un_narrow(nptr, ns, e);
    free(ns);
    return v;
}

/* ── time ─────────────────────────────────────────────────────────────────── */

size_t wcsftime(wchar_t *s, size_t maxsize, const wchar_t *format,
                const struct tm *tm) {
    if (maxsize == 0) return 0;
    char *nf = narrow_dup(format);
    if (!nf) return 0;
    char *nb = malloc(maxsize);       /* one wide unit == one byte here */
    if (!nb) { free(nf); return 0; }
    size_t r = strftime(nb, maxsize, nf, tm);
    free(nf);
    if (r == 0) { free(nb); return 0; }
    for (size_t i = 0; i <= r; i++) s[i] = (wchar_t)(unsigned char)nb[i];
    free(nb);
    return r;
}

/* ── wide streams ─────────────────────────────────────────────────────────── */

wint_t fgetwc(FILE *stream) {
    int c = fgetc(stream);
    if (c == EOF) return WEOF;
    if ((unsigned char)c > 0x7f) { errno = EILSEQ; return WEOF; }
    return (wint_t)c;
}

wint_t getwc(FILE *stream) { return fgetwc(stream); }

wint_t fputwc(wchar_t wc, FILE *stream) {
    if ((unsigned int)wc > 0x7f) { errno = EILSEQ; return WEOF; }
    if (fputc((int)wc, stream) == EOF) return WEOF;
    return (wint_t)wc;
}

wint_t putwc(wchar_t wc, FILE *stream) { return fputwc(wc, stream); }

wint_t ungetwc(wchar_t wc, FILE *stream) {
    if ((unsigned int)wc > 0x7f) { errno = EILSEQ; return WEOF; }
    if (ungetc((int)wc, stream) == EOF) return WEOF;
    return (wint_t)wc;
}

wint_t getwchar(void) { return fgetwc(stdin); }
wint_t putwchar(wchar_t wc) { return fputwc(wc, stdout); }

wchar_t *fgetws(wchar_t *s, int n, FILE *stream) {
    if (n <= 0) return NULL;
    int i = 0;
    while (i < n - 1) {
        wint_t c = fgetwc(stream);
        if (c == WEOF) { if (i == 0) return NULL; break; }
        s[i++] = (wchar_t)c;
        if (c == L'\n') break;
    }
    s[i] = L'\0';
    return s;
}

int fputws(const wchar_t *s, FILE *stream) {
    for (; *s; s++)
        if (fputwc(*s, stream) == WEOF) return -1;
    return 0;
}

/* CactOS's FILE carries no orientation flag (a byte stream has no need for one),
 * so the orientation is always "unset" — 0. */
int fwide(FILE *stream, int mode) {
    (void)stream;
    (void)mode;
    return 0;
}

/* ── wide formatted output ────────────────────────────────────────────────────
 * The format string is narrowed and the byte formatter does the work; its
 * ASCII result is widened.  See the header for what is not claimed. */
int vfwprintf(FILE *stream, const wchar_t *format, va_list ap) {
    char *nf = narrow_dup(format);
    if (!nf) return -1;
    int r = vfprintf(stream, nf, ap);
    free(nf);
    return r;
}

int fwprintf(FILE *stream, const wchar_t *format, ...) {
    va_list ap;
    va_start(ap, format);
    int r = vfwprintf(stream, format, ap);
    va_end(ap);
    return r;
}

int vwprintf(const wchar_t *format, va_list ap) {
    return vfwprintf(stdout, format, ap);
}

int wprintf(const wchar_t *format, ...) {
    va_list ap;
    va_start(ap, format);
    int r = vfwprintf(stdout, format, ap);
    va_end(ap);
    return r;
}

int vswprintf(wchar_t *s, size_t n, const wchar_t *format, va_list ap) {
    if (n == 0) return -1;
    char *nf = narrow_dup(format);
    if (!nf) return -1;
    char *nb = malloc(n);             /* one wide unit == one byte here */
    if (!nb) { free(nf); return -1; }
    int r = vsnprintf(nb, n, nf, ap);
    free(nf);
    if (r < 0 || (size_t)r >= n) { free(nb); return -1; }
    for (size_t i = 0; i <= (size_t)r; i++) s[i] = (wchar_t)(unsigned char)nb[i];
    free(nb);
    return r;
}

int swprintf(wchar_t *s, size_t n, const wchar_t *format, ...) {
    va_list ap;
    va_start(ap, format);
    int r = vswprintf(s, n, format, ap);
    va_end(ap);
    return r;
}

/* ── POSIX: display width ─────────────────────────────────────────────────────
 * ASCII only: a printable character is one column, NUL is none, everything else
 * (control, and anything above 0x7f, which is not in the C locale's set) is
 * "not printable", -1. */
int wcwidth(wchar_t wc) {
    if (wc == 0) return 0;
    if ((unsigned int)wc > 0x7e || (unsigned int)wc < 0x20) return -1;
    return 1;
}

int wcswidth(const wchar_t *s, size_t n) {
    int total = 0;
    for (size_t i = 0; i < n && s[i]; i++) {
        int w = wcwidth(s[i]);
        if (w < 0) return -1;
        total += w;
    }
    return total;
}
