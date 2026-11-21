#include "wctype.h"
#include "ctype.h"
#include "string.h"

/* <wctype.h> for the C locale.  CactOS has one locale, so classification and
 * case mapping are ASCII: a code unit above 0x7f is in no class and maps to
 * itself.  The byte classifiers in src/ctype.c do the work. */

static int isw_ascii(int (*byte_class)(int), wint_t wc) {
    if (wc == WEOF || wc > 0x7f) return 0;
    return byte_class((int)wc) != 0;
}

int iswalnum(wint_t wc) { return isw_ascii(isalnum, wc); }
int iswalpha(wint_t wc) { return isw_ascii(isalpha, wc); }
int iswblank(wint_t wc) { return isw_ascii(isblank, wc); }
int iswcntrl(wint_t wc) { return isw_ascii(iscntrl, wc); }
int iswdigit(wint_t wc) { return isw_ascii(isdigit, wc); }
int iswgraph(wint_t wc) { return isw_ascii(isgraph, wc); }
int iswlower(wint_t wc) { return isw_ascii(islower, wc); }
int iswprint(wint_t wc) { return isw_ascii(isprint, wc); }
int iswpunct(wint_t wc) { return isw_ascii(ispunct, wc); }
int iswspace(wint_t wc) { return isw_ascii(isspace, wc); }
int iswupper(wint_t wc) { return isw_ascii(isupper, wc); }
int iswxdigit(wint_t wc) { return isw_ascii(isxdigit, wc); }

wint_t towlower(wint_t wc) {
    if (wc == WEOF || wc > 0x7f) return wc;
    return (wint_t)tolower((int)wc);
}

wint_t towupper(wint_t wc) {
    if (wc == WEOF || wc > 0x7f) return wc;
    return (wint_t)toupper((int)wc);
}

/* wctype()/wctrans() names, as small indices — a descriptor is just "which
 * class/transformation", there being one locale. */
enum {
    WP_NONE = 0,
    WP_ALNUM, WP_ALPHA, WP_BLANK, WP_CNTRL, WP_DIGIT, WP_GRAPH,
    WP_LOWER, WP_PRINT, WP_PUNCT, WP_SPACE, WP_UPPER, WP_XDIGIT
};

wctype_t wctype(const char *property) {
    static const struct { const char *name; wctype_t prop; } tab[] = {
        { "alnum",  WP_ALNUM  }, { "alpha",  WP_ALPHA  },
        { "blank",  WP_BLANK  }, { "cntrl",  WP_CNTRL  },
        { "digit",  WP_DIGIT  }, { "graph",  WP_GRAPH  },
        { "lower",  WP_LOWER  }, { "print",  WP_PRINT  },
        { "punct",  WP_PUNCT  }, { "space",  WP_SPACE  },
        { "upper",  WP_UPPER  }, { "xdigit", WP_XDIGIT },
    };
    for (unsigned i = 0; i < sizeof(tab) / sizeof(tab[0]); i++)
        if (strcmp(property, tab[i].name) == 0) return tab[i].prop;
    return WP_NONE;
}

int iswctype(wint_t wc, wctype_t desc) {
    switch (desc) {
    case WP_ALNUM:  return iswalnum(wc);
    case WP_ALPHA:  return iswalpha(wc);
    case WP_BLANK:  return iswblank(wc);
    case WP_CNTRL:  return iswcntrl(wc);
    case WP_DIGIT:  return iswdigit(wc);
    case WP_GRAPH:  return iswgraph(wc);
    case WP_LOWER:  return iswlower(wc);
    case WP_PRINT:  return iswprint(wc);
    case WP_PUNCT:  return iswpunct(wc);
    case WP_SPACE:  return iswspace(wc);
    case WP_UPPER:  return iswupper(wc);
    case WP_XDIGIT: return iswxdigit(wc);
    default:        return 0;
    }
}

enum { WT_NONE = 0, WT_LOWER, WT_UPPER };

wctrans_t wctrans(const char *property) {
    if (strcmp(property, "tolower") == 0) return WT_LOWER;
    if (strcmp(property, "toupper") == 0) return WT_UPPER;
    return WT_NONE;
}

wint_t towctrans(wint_t wc, wctrans_t desc) {
    switch (desc) {
    case WT_LOWER: return towlower(wc);
    case WT_UPPER: return towupper(wc);
    default:       return wc;
    }
}
