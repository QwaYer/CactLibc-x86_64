/* <sys/utsname.h> forwarder — see sys/types.h.  POSIX spells the uname()
 * header this way; CactLibc keeps struct utsname and uname() flat in unistd.h
 * (uname() reads /proc/uname). */
#ifndef _CACT_SYS_UTSNAME_H
#define _CACT_SYS_UTSNAME_H
#include "../unistd.h"
#endif
