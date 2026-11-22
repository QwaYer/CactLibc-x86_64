#ifndef _WLAN_H
#define _WLAN_H

/* C linkage for the C++ consumers (libc++, LLVM, Mesa) that include this
 * header; a mangled reference would match no CactLibc export. */
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "ioctl_abi.h"

/*
 * Thin helpers over the /dev/wlan0 ioctls (CACT_WLANCTL_*).
 *
 * The 802.11 station engine runs in the kernel (cact80211): it scans, runs
 * open-system auth/association and the 802.11<->802.3 datapath with CCMP.  The
 * intended caller is wljoin — the WPA2 supplicant — which reads the scan list,
 * asks the kernel to join a BSS, exchanges EAPOL and installs the keys.  All
 * return 0 on success and -1 with errno set on failure unless noted.
 */

/* Rescan; returns the AP count (>= 0) or -1. */
int wl_scan(int fd);

/* Fetch BSS `bss->index` from the engine's scan list into *bss (index in). */
int wl_get_bss(int fd, cact_wlan_bss_t *bss);

/* Join a BSS: the engine runs open-system auth + association. */
int wl_connect(int fd, const uint8_t bssid[6], const uint8_t *ssid,
               uint8_t ssid_len);

/* Drop the association. */
int wl_disconnect(int fd);

/* Enable/disable the repeater (WDS, 4-address) frame mode: for an uplink that
 * is a repeater/WDS peer call wl_set_wds(fd, 1) *before* wl_connect(), so the
 * very first uplink frame is 4-address.  Returns 0 or -1. */
int wl_set_wds(int fd, int on);

/* 1/0 when the engine is speaking 4-address (repeater/WDS) frames now, -1 on
 * error. */
int wl_get_wds(int fd);

/* kind = CACT_WLAN_KEY_PAIRWISE (TK) or CACT_WLAN_KEY_GROUP (GTK); key_id is
 * the GTK key id the AP assigned (ignored for the pairwise key). */
int wl_set_key(int fd, uint32_t kind, uint32_t key_id, const void *key,
               uint32_t key_len);

int wl_status(int fd, cact_wlan_status_t *st);

/* Send one EAPOL payload to `dst` (the engine frames it as a ToDS data frame). */
int wl_eapol_tx(int fd, const uint8_t dst[6], const void *eapol, uint32_t len);

/* Fetch one received EAPOL payload into `eapol` (cap *len), setting *len to the
 * byte count and `src` (optional) to the sender.  Returns 1 when a frame was
 * copied, 0 when none is queued, -1 on error. */
int wl_eapol_rx(int fd, void *eapol, uint32_t *len, uint8_t src[6]);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* _WLAN_H */
