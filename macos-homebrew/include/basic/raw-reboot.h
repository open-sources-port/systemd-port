/* SPDX-License-Identifier: LGPL-2.1-or-later */
#pragma once

#ifndef RAW_REBOOT_H
#define RAW_REBOOT_H

#include <unistd.h>
#include <sys_compat/errno.h>
#include <sys/reboot.h>
#include <sys/syscall.h>
#include <stdlib.h>

// Supported commands
#define RAW_REBOOT_CMD_RESTART     1
#define RAW_REBOOT_CMD_POWER_OFF   2
#define RAW_REBOOT_CMD_HALT        3
#define RAW_REBOOT_CMD_SHUTDOWN    4
#define RAW_REBOOT_CMD_RESTART2 5

static inline int raw_reboot(int cmd, const void *arg) {
    (void)arg; // unused on macOS

    switch(cmd) {
        case RAW_REBOOT_CMD_RESTART:
            return reboot(RB_AUTOBOOT);

        case RAW_REBOOT_CMD_POWER_OFF:
            return reboot(RB_HALT);

        case RAW_REBOOT_CMD_HALT:
            return system("halt");

        case RAW_REBOOT_CMD_SHUTDOWN:
            return system("shutdown -h now");

        case RAW_REBOOT_CMD_RESTART2:
            // macOS cannot pass a boot string — return not implemented
            return -ENOSYS;

        default:
            return -ENOSYS;
    }
}

#endif // RAW_REBOOT_H