#pragma once

#ifndef __linux__
#include <sys/socket.h>   // for sa_family_t, struct sockaddr

#define AF_VSOCK 40   /* placeholder: real AF_VSOCK only exists on Linux */

struct sockaddr_vm {
    sa_family_t     svm_family;
    unsigned short  svm_reserved1;
    unsigned int    svm_port;
    unsigned int    svm_cid;
    unsigned char   svm_zero[
        sizeof(struct sockaddr)
        - sizeof(sa_family_t)
        - sizeof(unsigned short)
        - sizeof(unsigned int)
        - sizeof(unsigned int)
    ];
};

#endif /* __linux__ */
