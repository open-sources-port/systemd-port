/* bpf_socket.c - implementation using /dev/bpf* and ioctl on macOS */
#include "bpf_socket.h"

#ifdef __APPLE__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <net/bpf.h>
#include <string.h>
#include <ifaddrs.h>
#include <arpa/inet.h>

/* Open a free /dev/bpf device */
static int open_bpf_device(void) {
    int fd = -1;
    char devname[16];
    for (int i = 0; i < 256; i++) {
        snprintf(devname, sizeof(devname), "/dev/bpf%d", i);
        fd = open(devname, O_RDWR);
        if (fd >= 0) return fd;
        if (errno == EBUSY) continue;
        /* if other error (ENOENT/EACCES) keep trying few or bail */
    }
    return -1;
}

int bpf_open_on_interface(const char *ifname) {
    if (!ifname) { errno = EINVAL; return -1; }

    int fd = open_bpf_device();
    if (fd < 0) return -1;

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name) - 1);

    /* bind BPF to interface */
    if (ioctl(fd, BIOCSETIF, &ifr) == -1) {
        close(fd);
        return -1;
    }

    /* immediate mode: don't wait to fill buffer */
    u_int on = 1;
    if (ioctl(fd, BIOCIMMEDIATE, &on) == -1) {
        /* not fatal: continue */
    }

    /* optionally set header complete mode if you want to write full Ethernet frames */
    u_int hdr_complete = 1;
    if (ioctl(fd, BIOCSHDRCMPLT, &hdr_complete) == -1) {
        /* If unsupported, we'll still be able to write frames */
    }

    /* set non-blocking (optional) */
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);

    return fd;
}

ssize_t bpf_send_frame(int bpf_fd, const void *frame, size_t len) {
    if (bpf_fd < 0 || !frame || len == 0) { errno = EINVAL; return -1; }
    /* write() on bpf device sends the raw frame (when header complete) */
    return write(bpf_fd, frame, len);
}

/* Retrieve hardware/MAC address for interface name */
int bpf_get_hwaddr(const char *ifname, unsigned char out_mac[6]) {
    if (!ifname || !out_mac) { errno = EINVAL; return -1; }

    struct ifaddrs *ifap, *ifa;
    if (getifaddrs(&ifap) != 0) return -1;
    int rc = -1;

    for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) continue;
        if (strcmp(ifa->ifa_name, ifname) != 0) continue;
        if (ifa->ifa_addr->sa_family == AF_LINK) {
            struct sockaddr_dl *sdl = (struct sockaddr_dl *)ifa->ifa_addr;
            if (sdl->sdl_alen == 6) {
                unsigned char *addr = (unsigned char *)LLADDR(sdl);
                memcpy(out_mac, addr, 6);
                rc = 0;
                break;
            }
        }
    }
    freeifaddrs(ifap);
    return rc;
}

#else  /* non-Apple stub implementations */

int bpf_open_on_interface(const char *ifname) {
    (void)ifname;
    errno = ENOSYS;
    return -1;
}

int bpf_get_hwaddr(const char *ifname, unsigned char out_mac[6]) {
    (void)ifname; (void)out_mac;
    errno = ENOSYS;
    return -1;
}

ssize_t bpf_send_frame(int bpf_fd, const void *frame, size_t len) {
    (void)bpf_fd; (void)frame; (void)len;
    errno = ENOSYS;
    return -1;
}

#endif
