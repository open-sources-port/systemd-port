/* SPDX-License-Identifier: LGPL-2.1-or-later */
#pragma once

#include <inttypes.h>
#include <stddef.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <net/if_types.h>

const char *arphrd_to_name(int id);
int arphrd_from_name(const char *name);

size_t arphrd_to_hw_addr_len(uint16_t arphrd);

#ifndef GPERF_LEN_TYPE
#define GPERF_LEN_TYPE size_t
#endif

// MAC address length
#ifndef ETH_ALEN
#define ETH_ALEN 6
#endif

// Fallback ARPHRD values for macOS
#ifndef ARPHRD_INFINIBAND
#define ARPHRD_INFINIBAND 32
#endif

#ifndef ARPHRD_TUNNEL
#define ARPHRD_TUNNEL 131
#endif

#ifndef ARPHRD_SIT
#define ARPHRD_SIT 136
#endif

#ifndef ARPHRD_IPGRE
#define ARPHRD_IPGRE 148
#endif

#ifndef ARPHRD_TUNNEL6
#define ARPHRD_TUNNEL6 247
#endif

#ifndef ARPHRD_IP6GRE
#define ARPHRD_IP6GRE 253
#endif
