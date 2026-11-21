#ifndef _WCTYPE_H
#define _WCTYPE_H

/* C linkage for the C++ consumers (libc++, LLVM, Mesa) that include this
 * header; a mangled reference would match no CactLibc export. */
#ifdef __cplusplus
extern "C" {
#endif

/* C99 7.30 <wctype.h>, for the C locale.  CactOS has one locale, so wide
 * classification and case mapping are the ASCII ones — anything above 0x7f is
 * not a letter/digit/… and maps to itself. */

#ifndef _CACT_WINT_T
#define _CACT_WINT_T
typedef unsigned int wint_t;
#endif

#ifndef WEOF
#define WEOF ((wint_t)-1)
#endif

/* A classification descriptor.  glibc makes wctype_t a locale pointer;
 * CactOS has a single locale, so it is a small property index (see
 * src/wctype.c).  wctrans_t likewise. */
typedef unsigned long wctype_t;
typedef int           wctrans_t;

int iswalnum(wint_t wc);
int iswalpha(wint_t wc);
int iswblank(wint_t wc);
int iswcntrl(wint_t wc);
int iswdigit(wint_t wc);
int iswgraph(wint_t wc);
int iswlower(wint_t wc);
int iswprint(wint_t wc);
int iswpunct(wint_t wc);
int iswspace(wint_t wc);
int iswupper(wint_t wc);
int iswxdigit(wint_t wc);

wctype_t wctype(const char *property);
int iswctype(wint_t wc, wctype_t desc);
wctrans_t wctrans(const char *property);
wint_t towctrans(wint_t wc, wctrans_t desc);

wint_t towlower(wint_t wc);
wint_t towupper(wint_t wc);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* _WCTYPE_H */
