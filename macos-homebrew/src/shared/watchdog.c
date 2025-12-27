/* SPDX-License-Identifier: LGPL-2.1-or-later */

#include <stdbool.h>
#include <time.h>
#include <linux/rtc.h>

#include "log.h"
#include "time-util.h"
#include "watchdog.h"

/*
 * macOS does NOT support Linux-style hardware watchdogs.
 * This is a stub implementation that preserves systemd semantics:
 *  - All setup calls succeed
 *  - watchdog is treated as unsupported
 *  - ping is a no-op
 */

static usec_t watchdog_timeout = 0;
static usec_t watchdog_last_ping = USEC_INFINITY;

const char *watchdog_get_device(void) {
        return NULL;
}

int watchdog_set_device(const char *path) {
        /* No watchdog devices on macOS */
        (void) path;
        return 0;
}

int watchdog_setup(usec_t timeout) {
        /* timeout=0 disables watchdog */
        watchdog_timeout = timeout;
        watchdog_last_ping = now(CLOCK_BOOTTIME);
        return 0;
}

int watchdog_setup_pretimeout(usec_t timeout) {
        /* Pretimeout unsupported on macOS */
        (void) timeout;
        return 0;
}

int watchdog_setup_pretimeout_governor(const char *governor) {
        /* Pretimeout governors do not exist on macOS */
        (void) governor;
        return 0;
}

usec_t watchdog_get_last_ping(clockid_t clock) {
        return map_clock_usec(watchdog_last_ping, CLOCK_BOOTTIME, clock);
}

usec_t watchdog_runtime_wait(void) {
        /* No watchdog → nothing to wait for */
        return USEC_INFINITY;
}

int watchdog_ping(void) {
        /* No-op */
        watchdog_last_ping = now(CLOCK_BOOTTIME);
        return 0;
}

void watchdog_close(bool disarm) {
        (void) disarm;
        watchdog_timeout = 0;
        watchdog_last_ping = USEC_INFINITY;
}
