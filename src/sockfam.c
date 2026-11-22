/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 CactOS Project */

#include "sockfam.h"

/* fd numbers are small in this system (the kernel's per-process table is
 * bounded well below this), so a flat array is enough. */
#define SOCKFAM_MAX 256

static unsigned char g_fam[SOCKFAM_MAX];

static int inrange(int fd) {
    return fd >= 0 && fd < SOCKFAM_MAX;
}

void __cact_sockfam_set(int fd, int family) {
    if (inrange(fd))
        g_fam[fd] = (unsigned char)family;
}

void __cact_sockfam_clear(int fd) {
    if (inrange(fd))
        g_fam[fd] = 0;
}

int __cact_sockfam_get(int fd) {
    return inrange(fd) ? (int)g_fam[fd] : 0;
}

void __cact_sockfam_move(int oldfd, int newfd) {
    if (inrange(newfd))
        g_fam[newfd] = inrange(oldfd) ? g_fam[oldfd] : 0;
}
