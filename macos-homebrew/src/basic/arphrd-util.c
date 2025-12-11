/* SPDX-License-Identifier: LGPL-2.1-or-later */

#include <errno.h>
#include <netinet/in.h>
#include <net/if_arp.h>
#include <compat/linux_if_infiniband.h>
#include <string.h>

#include "arphrd-util.h"
#include <basic/macro.h>

static const struct arphrd_name* lookup_arphrd(register const char *str, register GPERF_LEN_TYPE len);

#include <compat/compat_arphrd_from_name.h>
#include <compat/compat_arphrd_to_name.h>

size_t arphrd_to_hw_addr_len(uint16_t arphrd) {
        switch (arphrd) {
        case ARPHRD_ETHER:
                return ETH_ALEN;
        case ARPHRD_INFINIBAND:
                return INFINIBAND_ALEN;
        case ARPHRD_TUNNEL:
        case ARPHRD_SIT:
        case ARPHRD_IPGRE:
                return sizeof(struct in_addr);
        case ARPHRD_TUNNEL6:
        case ARPHRD_IP6GRE:
                return sizeof(struct in6_addr);
        default:
                return 0;
        }
}
