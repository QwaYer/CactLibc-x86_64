#ifndef _CACT_SOCKFAM_H
#define _CACT_SOCKFAM_H

/* Per-fd address-family memo for the socket layer.  The kernel's AF_INET and
 * AF_INET6 data-path ioctls are separate (see CACT_SOCKCTL_*_6), and a few of
 * them — recvfrom, getsockname/getpeername, accept — carry no family in their
 * argument, so libc has to remember which family each socket was created with.
 * __cact_sockfam_get returns 0 for an fd that is not a known socket, which the
 * callers treat as "use the AF_INET path". */
void __cact_sockfam_set(int fd, int family);
void __cact_sockfam_clear(int fd);
int  __cact_sockfam_get(int fd);
/* After dup/dup2/dup3: the new fd inherits the old one's family. */
void __cact_sockfam_move(int oldfd, int newfd);

#endif /* _CACT_SOCKFAM_H */
