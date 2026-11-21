#ifndef _WCHAR_H
#define _WCHAR_H

/* C linkage for the C++ consumers (libc++, LLVM, Mesa) that include this
 * header; a mangled reference would match no CactLibc export. */
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>   /* size_t, wchar_t, NULL */
#include <stdarg.h>   /* va_list */
#include <stdio.h>    /* FILE */
#include <time.h>     /* struct tm */
#include <uchar.h>    /* mbstate_t */

/* C99 7.29 <wchar.h>, for the C locale.
 *
 * CactOS has exactly one encoding — the C locale's, a single-byte one — so a
 * multibyte character is one code unit, no conversion is ever stateful, and
 * `wchar_t` holds a byte value.  Bytes above 0x7f are not in the C locale's
 * character set, so they are EILSEQ rather than a silent identity mapping
 * (the same convention <uchar.h> uses).  `__WCHAR_TYPE__` is `int` on this
 * target, so the range below is the signed-32-bit one.
 *
 * Not provided, because CactOS is a C-locale libc and has no wide-scanning
 * engine: the wide *scanf* family (fwscanf/wscanf/swscanf and the v- forms).
 * A program that needs them gets a compile error here rather than a silent
 * misparse — libc++ binds those names with `using_if_exists`, so their absence
 * is not an error there. */

#ifndef _CACT_WINT_T
#define _CACT_WINT_T
typedef unsigned int wint_t;
#endif

#ifndef WEOF
#define WEOF ((wint_t)-1)
#endif

#ifndef WCHAR_MIN
#define WCHAR_MIN (-2147483647 - 1)
#define WCHAR_MAX  2147483647
#endif

/* ── wide formatted output ──────────────────────────────────────────────────
 * The C locale's `%f`/`%d`/`%s`(byte) output is ASCII, and one wide code unit
 * maps to one byte, so these format with the byte engine and widen the result.
 * An argument that a *wide* `wprintf` would read as wide — `%ls`, `%lc` — is
 * read as narrow here and produces wrong but bounded output; no wide-scanning
 * or wide-argument formatting is claimed. */
int fwprintf(FILE *stream, const wchar_t *format, ...);
int wprintf(const wchar_t *format, ...);
int swprintf(wchar_t *s, size_t n, const wchar_t *format, ...);
int vfwprintf(FILE *stream, const wchar_t *format, va_list ap);
int vwprintf(const wchar_t *format, va_list ap);
int vswprintf(wchar_t *s, size_t n, const wchar_t *format, va_list ap);

/* ── wide streams ───────────────────────────────────────────────────────────
 * CactOS's FILE is a byte stream; in the C locale a wide character is one byte,
 * so these are the byte operations with the EILSEQ check in front. */
wint_t fgetwc(FILE *stream);
wint_t getwc(FILE *stream);
wint_t fputwc(wchar_t wc, FILE *stream);
wint_t putwc(wchar_t wc, FILE *stream);
wint_t ungetwc(wchar_t wc, FILE *stream);
wint_t getwchar(void);
wint_t putwchar(wchar_t wc);
wchar_t *fgetws(wchar_t *s, int n, FILE *stream);
int fputws(const wchar_t *s, FILE *stream);
int fwide(FILE *stream, int mode);

/* ── conversions between the multibyte and wide encodings ──────────────────── */
wint_t btowc(int c);
int wctob(wint_t c);
int mbsinit(const mbstate_t *ps);
size_t mbrlen(const char *s, size_t n, mbstate_t *ps);
size_t mbrtowc(wchar_t *pwc, const char *s, size_t n, mbstate_t *ps);
size_t wcrtomb(char *s, wchar_t wc, mbstate_t *ps);
size_t mbsrtowcs(wchar_t *dst, const char **src, size_t len, mbstate_t *ps);
size_t wcsrtombs(char *dst, const wchar_t **src, size_t len, mbstate_t *ps);
/* POSIX (XSI) bounded forms — the byte/wide count in the middle is what libc++'s
 * locale layer binds to. */
size_t mbsnrtowcs(wchar_t *dst, const char **src, size_t nms, size_t len,
                  mbstate_t *ps);
size_t wcsnrtombs(char *dst, const wchar_t **src, size_t nwc, size_t len,
                  mbstate_t *ps);

/* ── wide strings ─────────────────────────────────────────────────────────── */
size_t wcslen(const wchar_t *s);
size_t wcsnlen(const wchar_t *s, size_t maxlen);
wchar_t *wcscpy(wchar_t *dst, const wchar_t *src);
wchar_t *wcsncpy(wchar_t *dst, const wchar_t *src, size_t n);
wchar_t *wcscat(wchar_t *dst, const wchar_t *src);
wchar_t *wcsncat(wchar_t *dst, const wchar_t *src, size_t n);
int wcscmp(const wchar_t *a, const wchar_t *b);
int wcsncmp(const wchar_t *a, const wchar_t *b, size_t n);
int wcscoll(const wchar_t *a, const wchar_t *b);
size_t wcsxfrm(wchar_t *dst, const wchar_t *src, size_t n);
wchar_t *wcschr(const wchar_t *s, wchar_t c);
wchar_t *wcsrchr(const wchar_t *s, wchar_t c);
wchar_t *wcsstr(const wchar_t *haystack, const wchar_t *needle);
wchar_t *wcspbrk(const wchar_t *s, const wchar_t *accept);
size_t wcsspn(const wchar_t *s, const wchar_t *accept);
size_t wcscspn(const wchar_t *s, const wchar_t *reject);
wchar_t *wcstok(wchar_t *s, const wchar_t *delim, wchar_t **saveptr);
wchar_t *wcsdup(const wchar_t *s);

/* ── wide memory ──────────────────────────────────────────────────────────── */
wchar_t *wmemchr(const wchar_t *s, wchar_t c, size_t n);
int wmemcmp(const wchar_t *a, const wchar_t *b, size_t n);
wchar_t *wmemcpy(wchar_t *dst, const wchar_t *src, size_t n);
wchar_t *wmemmove(wchar_t *dst, const wchar_t *src, size_t n);
wchar_t *wmemset(wchar_t *s, wchar_t c, size_t n);

/* ── wide numeric conversions ─────────────────────────────────────────────── */
long wcstol(const wchar_t *nptr, wchar_t **endptr, int base);
unsigned long wcstoul(const wchar_t *nptr, wchar_t **endptr, int base);
long long wcstoll(const wchar_t *nptr, wchar_t **endptr, int base);
unsigned long long wcstoull(const wchar_t *nptr, wchar_t **endptr, int base);
double wcstod(const wchar_t *nptr, wchar_t **endptr);
float wcstof(const wchar_t *nptr, wchar_t **endptr);
long double wcstold(const wchar_t *nptr, wchar_t **endptr);

/* ── wide time ────────────────────────────────────────────────────────────── */
size_t wcsftime(wchar_t *s, size_t maxsize, const wchar_t *format,
                const struct tm *tm);

/* ── POSIX: display width ─────────────────────────────────────────────────── */
int wcwidth(wchar_t wc);
int wcswidth(const wchar_t *s, size_t n);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* _WCHAR_H */
