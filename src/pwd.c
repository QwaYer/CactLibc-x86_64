#include "pwd.h"

/* <pwd.h> on a system with no user database: every lookup finds nothing.  See
 * the header for why that is the true answer rather than a failure. */

struct passwd *getpwuid(uid_t uid) {
    (void)uid;
    return NULL;
}

struct passwd *getpwnam(const char *name) {
    (void)name;
    return NULL;
}

int getpwuid_r(uid_t uid, struct passwd *pwd, char *buf, size_t buflen,
               struct passwd **result) {
    (void)uid; (void)pwd; (void)buf; (void)buflen;
    if (result) *result = NULL;
    return 0;   /* "not found" is not an error */
}

int getpwnam_r(const char *name, struct passwd *pwd, char *buf, size_t buflen,
               struct passwd **result) {
    (void)name; (void)pwd; (void)buf; (void)buflen;
    if (result) *result = NULL;
    return 0;
}
