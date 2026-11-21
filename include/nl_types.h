#ifndef _NL_TYPES_H
#define _NL_TYPES_H

/* C linkage for the C++ consumers (libc++, LLVM, Mesa) that include this
 * header; a mangled reference would match no CactLibc export. */
#ifdef __cplusplus
extern "C" {
#endif

/* POSIX.1-2008 XBD 13 <nl_types.h> — message catalogues.
 *
 * CactOS has no message catalogues and no catalogue files to open, so the
 * functions are the "no catalogues here" implementation a C library is allowed
 * to have: catopen() fails, and catgets() then only ever returns the caller's
 * default string (which is what the standard says to do when a message is
 * missing).  The header exists because a C++ standard library's std::messages
 * facet is built on it. */

typedef int  nl_item;
typedef void *nl_catd;          /* (nl_catd)-1 is the invalid catalogue */

#define NL_SETD       1
#define NL_CAT_LOCALE 1

nl_catd catopen(const char *name, int oflag);
int     catclose(nl_catd catalog);
char   *catgets(nl_catd catalog, int set_id, int msg_id, const char *s);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
