/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 CactOS Project */

/* inet_pton/inet_ntop for AF_INET and AF_INET6 — the textual <-> binary address
 * conversion userspace needs for `ip -6` and `ping6`.  The parsing is strict:
 * an address with trailing junk, a missing group or a doubled "::" is rejected
 * (return 0), never silently truncated. */

#include "socket.h"
#include "errno.h"
#include <stdint.h>
#include <string.h>

/* ── AF_INET ─────────────────────────────────────────────────────────────── */

static int parse_v4(const char *s, uint8_t out[4]) {
    for (int part = 0; part < 4; part++) {
        if (*s < '0' || *s > '9')
            return 0;
        int v = 0, digits = 0;
        while (*s >= '0' && *s <= '9') {
            v = v * 10 + (*s - '0');
            if (++digits > 3 || v > 255)
                return 0;
            s++;
        }
        out[part] = (uint8_t)v;
        if (part < 3) {
            if (*s != '.')
                return 0;
            s++;
        }
    }
    return *s == '\0';
}

static void fmt_v4(const uint8_t in[4], char *dst) {
    char *p = dst;
    for (int i = 0; i < 4; i++) {
        unsigned v = in[i];
        char tmp[3];
        int n = 0;
        if (v == 0) {
            tmp[n++] = '0';
        } else {
            while (v) {
                tmp[n++] = (char)('0' + v % 10);
                v /= 10;
            }
        }
        while (n)
            *p++ = tmp[--n];
        if (i < 3)
            *p++ = '.';
    }
    *p = '\0';
}

/* ── AF_INET6 ────────────────────────────────────────────────────────────── */

static int hexval(int c) {
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return -1;
}

/* Parse one 1..4 digit hex group, advancing *sp; returns the value or -1. */
static int parse_group(const char **sp) {
    const char *s = *sp;
    int v = 0, n = 0, c;
    while (n < 4 && (c = hexval((unsigned char)*s)) >= 0) {
        v = v * 16 + c;
        s++;
        n++;
    }
    if (n == 0)
        return -1;
    *sp = s;
    return v;
}

static int parse_v6(const char *s, uint8_t out[16]) {
    uint16_t grp[8];
    int ng = 0;      /* groups parsed */
    int dcolon = -1; /* group index the "::" run starts at */

    if (s[0] == ':') {
        if (s[1] != ':')
            return 0;
        dcolon = 0;
        s += 2;
    }
    while (*s) {
        if (ng >= 8)
            return 0;
        int v = parse_group(&s);
        if (v < 0)
            return 0;
        grp[ng++] = (uint16_t)v;
        if (*s == ':') {
            s++;
            if (*s == ':') {
                if (dcolon >= 0)
                    return 0; /* a second "::" */
                dcolon = ng;
                s++;
                if (*s == '\0')
                    break;
            } else if (*s == '\0') {
                return 0; /* trailing single colon */
            }
        } else if (*s != '\0') {
            return 0;
        }
    }

    uint8_t bytes[16];
    int w = 0;
    if (dcolon < 0) {
        if (ng != 8)
            return 0;
        for (int i = 0; i < 8; i++) {
            bytes[w++] = (uint8_t)(grp[i] >> 8);
            bytes[w++] = (uint8_t)(grp[i] & 0xff);
        }
    } else {
        int fill = 8 - ng;
        if (fill < 1)
            return 0; /* "::" must stand for at least one zero group */
        for (int i = 0; i < dcolon; i++) {
            bytes[w++] = (uint8_t)(grp[i] >> 8);
            bytes[w++] = (uint8_t)(grp[i] & 0xff);
        }
        for (int i = 0; i < fill; i++) {
            bytes[w++] = 0;
            bytes[w++] = 0;
        }
        for (int i = dcolon; i < ng; i++) {
            bytes[w++] = (uint8_t)(grp[i] >> 8);
            bytes[w++] = (uint8_t)(grp[i] & 0xff);
        }
    }
    if (w != 16)
        return 0;
    memcpy(out, bytes, 16);
    return 1;
}

static void fmt_group(char **pp, unsigned v) {
    char *p = *pp;
    char tmp[4];
    int n = 0;
    if (v == 0) {
        tmp[n++] = '0';
    } else {
        while (v) {
            int d = (int)(v & 0xf);
            tmp[n++] = (char)(d < 10 ? '0' + d : 'a' + d - 10);
            v >>= 4;
        }
    }
    while (n)
        *p++ = tmp[--n];
    *pp = p;
}

static void fmt_v6(const uint8_t in[16], char *dst) {
    uint16_t g[8];
    for (int i = 0; i < 8; i++)
        g[i] = (uint16_t)((in[2 * i] << 8) | in[2 * i + 1]);

    /* Longest run of zero groups; only a run of two or more is compressed. */
    int best = -1, bestlen = 0;
    for (int i = 0; i < 8;) {
        if (g[i] == 0) {
            int j = i;
            while (j < 8 && g[j] == 0)
                j++;
            int len = j - i;
            if (len > bestlen) {
                bestlen = len;
                best = i;
            }
            i = j;
        } else {
            i++;
        }
    }
    if (bestlen < 2)
        best = -1;

    char *p = dst;
    int first = 1;
    int i = 0;
    while (i < 8) {
        if (i == best) {
            *p++ = ':';
            *p++ = ':';
            i += bestlen;
            first = 0;
            continue;
        }
        if (!first)
            *p++ = ':';
        fmt_group(&p, g[i]);
        first = 0;
        i++;
    }
    *p = '\0';
}

/* ── Public API ──────────────────────────────────────────────────────────── */

int inet_pton(int af, const char *src, void *dst) {
    if (!src || !dst) {
        errno = EINVAL;
        return -1;
    }
    if (af == AF_INET)
        return parse_v4(src, (uint8_t *)dst);
    if (af == AF_INET6)
        return parse_v6(src, (uint8_t *)dst);
    errno = EAFNOSUPPORT;
    return -1;
}

const char *inet_ntop(int af, const void *src, char *dst, uint32_t size) {
    if (!src || !dst) {
        errno = EINVAL;
        return 0;
    }
    if (af == AF_INET) {
        if (size < 16) {
            errno = ENOSPC;
            return 0;
        }
        fmt_v4((const uint8_t *)src, dst);
        return dst;
    }
    if (af == AF_INET6) {
        if (size < 46) {
            errno = ENOSPC;
            return 0;
        }
        fmt_v6((const uint8_t *)src, dst);
        return dst;
    }
    errno = EAFNOSUPPORT;
    return 0;
}
