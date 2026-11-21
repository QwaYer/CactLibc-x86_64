#include "nl_types.h"

/* There are no message catalogues on a CactOS image and no catalogue files to
 * open (see <nl_types.h>).  catopen() reports failure, catgets() hands back the
 * caller's default, and errno is left alone: a missing catalogue is not an
 * error, it is the only outcome. */

nl_catd catopen(const char *name, int oflag) {
    (void)name;
    (void)oflag;
    return (nl_catd)-1;
}

int catclose(nl_catd catalog) {
    return (catalog == (nl_catd)-1) ? -1 : 0;
}

char *catgets(nl_catd catalog, int set_id, int msg_id, const char *s) {
    (void)catalog;
    (void)set_id;
    (void)msg_id;
    return (char *)s;
}
