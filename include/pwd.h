#ifndef _PWD_H
#define _PWD_H

/* C linkage for the C++ consumers (libc++, LLVM, Mesa) that include this
 * header; a mangled reference would match no CactLibc export. */
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include "unistd.h"   /* uid_t, gid_t */

/* CactOS has no user database — one user (root), no /etc/passwd.  The lookups
 * therefore succeed at the call but find nothing: getpwuid_r()/getpwnam_r()
 * set *result to NULL, and getpwuid()/getpwnam() return NULL.  A ported program
 * that expands `~` (LLVM does) falls back to $HOME, which is the correct
 * behaviour on a system with no passwd entries. */

struct passwd {
    char  *pw_name;
    char  *pw_passwd;
    uid_t  pw_uid;
    gid_t  pw_gid;
    char  *pw_gecos;
    char  *pw_dir;
    char  *pw_shell;
};

struct passwd *getpwuid(uid_t uid);
struct passwd *getpwnam(const char *name);
int getpwuid_r(uid_t uid, struct passwd *pwd, char *buf, size_t buflen,
               struct passwd **result);
int getpwnam_r(const char *name, struct passwd *pwd, char *buf, size_t buflen,
               struct passwd **result);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* _PWD_H */
