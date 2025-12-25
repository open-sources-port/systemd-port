/* SPDX-License-Identifier: LGPL-2.1-or-later */
#include "process-util.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <pthread.h>
#include <limits.h>
#include <libproc.h>
#include <sys/sysctl.h>

/* ----------------------- raw_clone wrapper ----------------------- */
inline pid_t raw_clone(unsigned long flags) {
    (void)flags;
    return fork();
}

/* ----------------------- program name ----------------------- */
const char *program_invocation_short_name_fallback(void) {
    return getprogname();
}

/* ----------------------- wait helpers ----------------------- */
int wait_for_terminate(pid_t pid, siginfo_t *status) {
    if (!pid_is_valid(pid)) return -1;

    int wstatus;
    int ret = waitpid(pid, &wstatus, 0);
    if (ret > 0 && status) {
        memset(status, 0, sizeof(siginfo_t));
        status->si_pid = pid;
        if (WIFEXITED(wstatus)) {
            status->si_code = CLD_EXITED;
            status->si_status = WEXITSTATUS(wstatus);
        } else if (WIFSIGNALED(wstatus)) {
            status->si_code = CLD_KILLED;
            status->si_status = WTERMSIG(wstatus);
        }
    }
    return ret;
}

void sigkill_wait(pid_t pid) {
    if (!pid_is_valid(pid)) return;
    kill(pid, SIGKILL);
    wait_for_terminate(pid, NULL);
}

void sigterm_wait(pid_t pid) {
    if (!pid_is_valid(pid)) return;
    kill(pid, SIGTERM);
    wait_for_terminate(pid, NULL);
}

void sigkill_waitp(pid_t *pid) {
    if (!pid || !pid_is_valid(*pid)) return;
    sigkill_wait(*pid);
    *pid = 0;
}

/* ----------------------- fork helpers ----------------------- */
int safe_fork_full(const char *name, const int except_fds[], size_t n_except_fds,
                   ForkFlags flags, pid_t *ret_pid) {
    (void)name; (void)except_fds; (void)n_except_fds; (void)flags;

    pid_t pid = fork();
    if (pid < 0) return -errno;
    if (ret_pid) *ret_pid = pid;
    return 0;
}

/* ----------------------- process information ----------------------- */
int get_process_comm(pid_t pid, char **ret) {
    if (!pid_is_valid(pid)) return -EINVAL;
    static char name[PROC_PIDPATHINFO_MAXSIZE];
    int len = proc_name(pid, name, sizeof(name));
    if (len <= 0) return -errno;
    *ret = strdup(name);
    return 0;
}

int get_process_cmdline(pid_t pid, size_t max_columns, ProcessCmdlineFlags flags, char **ret) {
    (void)max_columns; (void)flags;
    if (!pid_is_valid(pid)) return -EINVAL;
    char pathbuf[PROC_PIDPATHINFO_MAXSIZE];
    if (proc_pidpath(pid, pathbuf, sizeof(pathbuf)) <= 0) return -errno;
    *ret = strdup(pathbuf);
    return 0;
}

int get_process_exe(pid_t pid, char **ret) {
    if (!pid_is_valid(pid)) return -EINVAL;
    char pathbuf[PROC_PIDPATHINFO_MAXSIZE];
    if (proc_pidpath(pid, pathbuf, sizeof(pathbuf)) <= 0) return -errno;
    *ret = strdup(pathbuf);
    return 0;
}

int get_process_ppid(pid_t pid, pid_t *ret) {
    if (!pid_is_valid(pid)) return -EINVAL;
    struct kinfo_proc info;
    size_t size = sizeof(info);
    int mib[4] = { CTL_KERN, KERN_PROC, KERN_PROC_PID, pid };
    if (sysctl(mib, 4, &info, &size, NULL, 0) == -1) return -errno;
    *ret = info.kp_eproc.e_ppid;
    return 0;
}

int get_process_uid(pid_t pid, uid_t *ret) {
    if (!pid_is_valid(pid)) return -EINVAL;
    struct kinfo_proc info;
    size_t size = sizeof(info);
    int mib[4] = { CTL_KERN, KERN_PROC, KERN_PROC_PID, pid };
    if (sysctl(mib, 4, &info, &size, NULL, 0) == -1) return -errno;
    *ret = info.kp_eproc.e_ucred.cr_uid;
    return 0;
}

int get_process_gid(pid_t pid, gid_t *ret) {
    if (!pid_is_valid(pid)) return -EINVAL;
    struct kinfo_proc info;
    size_t size = sizeof(info);
    int mib[4] = { CTL_KERN, KERN_PROC, KERN_PROC_PID, pid };
    if (sysctl(mib, 4, &info, &size, NULL, 0) == -1) return -errno;
    *ret = info.kp_eproc.e_ucred.cr_gid;
    return 0;
}

/* ----------------------- cwd & root ----------------------- */
int get_process_cwd(pid_t pid, char **ret) {
    if (!pid_is_valid(pid)) return -EINVAL;
    struct proc_vnodepathinfo vpi;
    if (proc_pidinfo(pid, PROC_PIDVNODEPATHINFO, 0, &vpi, sizeof(vpi)) <= 0)
        return -errno;
    *ret = strdup(vpi.pvi_cdir.vip_path);
    return 0;
}

int get_process_root(pid_t pid, char **ret) {
    if (!pid_is_valid(pid)) return -EINVAL;
    struct proc_vnodepathinfo vpi;
    if (proc_pidinfo(pid, PROC_PIDVNODEPATHINFO, 0, &vpi, sizeof(vpi)) <= 0)
        return -errno;
    *ret = strdup(vpi.pvi_rdir.vip_path);
    return 0;
}

int get_process_environ(pid_t pid, char **ret) {
    if (!pid_is_valid(pid)) return -EINVAL;

    if (pid != getpid()) {
        *ret = NULL;
        return -ENOSYS; /* Only current process supported */
    }

    extern char **environ;
    size_t total_len = 0;
    size_t count = 0;

    /* Calculate total length */
    for (char **p = environ; *p; p++) {
        total_len += strlen(*p) + 1; /* +1 for null terminator */
        count++;
    }

    char *buf = malloc(total_len);
    if (!buf) return -ENOMEM;

    char *ptr = buf;
    for (char **p = environ; *p; p++) {
        size_t len = strlen(*p) + 1;
        memcpy(ptr, *p, len);
        ptr += len;
    }

    *ret = buf;
    return (int)count; /* return number of entries */
}

/* ----------------------- misc ----------------------- */
bool pid_is_alive(pid_t pid) {
    return (pid > 0 && kill(pid, 0) == 0);
}

bool pid_is_unwaited(pid_t pid) { (void)pid; return false; }
int pid_is_my_child(pid_t pid) { (void)pid; return -ENOSYS; }
int pid_from_same_root_fs(pid_t pid) { (void)pid; return -ENOSYS; }
bool is_main_thread(void) { return pthread_main_np() != 0; }
