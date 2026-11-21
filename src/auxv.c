/*
 * getauxval(3).
 *
 * CactOS's kernel hands a program no auxiliary vector — the exec entry carries
 * argc/argv/envp and nothing after — so no AT_* value exists.  Report ENOENT
 * for every key, which is glibc's answer for a key the vector does not carry.
 */

#include "sys/auxv.h"
#include "errno.h"

unsigned long getauxval(unsigned long type) {
    (void)type;
    errno = ENOENT;
    return 0;
}
