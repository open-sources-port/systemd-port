/* bpf_socket.h - small helpers for raw Ethernet I/O on macOS using BPF */
#pragma once

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Open and bind a BPF device to an interface name (e.g. "en0").
 * Returns file descriptor >=0 on success, -1 on error (errno set).
 *
 * The returned fd is ready to read() raw frames; use write() to send
 * frames that include the full Ethernet header.
 */
int bpf_open_on_interface(const char *ifname);

/* Get the data link (MAC) address of an interface.
 * Returns 0 on success, -1 on failure.
 */
int bpf_get_hwaddr(const char *ifname, unsigned char out_mac[6]);

/* Send an Ethernet frame using a BPF fd. Returns number of bytes written or -1. */
ssize_t bpf_send_frame(int bpf_fd, const void *frame, size_t len);

#ifdef __cplusplus
}
#endif
