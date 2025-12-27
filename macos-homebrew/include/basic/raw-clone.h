/* SPDX-License-Identifier: LGPL-2.1-or-later */
#pragma once

/***
  Copyright © 2016 Michael Karcher
***/

#include <errno.h>
#include <sched.h>
#include <sys/syscall.h>

#include "log.h"
#include <basic/macro.h>
#include "missing_sched.h"

/**
 * raw_clone() - uses clone to create a new process with clone flags
 * @flags: Flags to pass to the clone system call
 *
 * Uses the clone system call to create a new process with the cloning flags and termination signal passed in the flags
 * parameter. Opposed to glibc's clone function, using this function does not set up a separate stack for the child, but
 * relies on copy-on-write semantics on the one stack at a common virtual address, just as fork does.
 *
 * To obtain copy-on-write semantics, flags must not contain CLONE_VM, and thus CLONE_THREAD and CLONE_SIGHAND (which
 * require CLONE_VM) are not usable.
 *
 * Additionally, as this function does not pass the ptid, newtls and ctid parameters to the kernel, flags must not
 * contain CLONE_PARENT_SETTID, CLONE_CHILD_SETTID, CLONE_CHILD_CLEARTID or CLONE_SETTLS.
 *
 * Returns: 0 in the child process and the child process id in the parent.
 */
static inline pid_t raw_clone(unsigned long flags) {
    (void)flags;
    return fork();
}
