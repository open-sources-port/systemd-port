/* SPDX-License-Identifier: LGPL-2.1-or-later */
#include "process-util.h"

#include <sys_compat/errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <pthread.h>
#include <limits.h>
#include <libproc.h>
#include <sys/sysctl.h>
#include <stdatomic.h>
#include <sys_compat/personality.h>
#include <sys_compat/glibc.h>

/* Converts a string to a POSIX scheduling policy.
 * Returns -EINVAL if unknown. */
int sched_policy_from_string(const char *s) {
    if (!s)
        return -EINVAL;

    if (strcmp(s, "other") == 0)
        return SCHED_OTHER;
#if defined(SCHED_FIFO)
    else if (strcmp(s, "fifo") == 0)
        return SCHED_FIFO;
#endif
#if defined(SCHED_RR)
    else if (strcmp(s, "rr") == 0)
        return SCHED_RR;
#endif
    else
        return -EINVAL; /* Unknown policy */
}

/* Compare two pid_t values for sorting or searching */
int pid_compare_func(const pid_t *a, const pid_t *b) {
    if (*a < *b)
        return -1;
    else if (*a > *b)
        return 1;
    else
        return 0;
}

/* macOS does not support pidfd, so we stub this function */
int pidfd_get_pid(int fd, pid_t *ret) {
    (void) fd;   /* unused */
    if (ret)
        *ret = -1;  /* indicate invalid PID */
    return -ENOSYS;  /* Function not implemented on macOS */
}

unsigned long personality_from_string(const char *p) {
    if (!p)
        return PER_LINUX;

    if (strcmp(p, "linux") == 0)
        return PER_LINUX;
    if (strcmp(p, "linux32") == 0)
        return PER_LINUX_32BIT;
    if (strcmp(p, "svr4") == 0)
        return PER_SVR4;
    if (strcmp(p, "scosvr4") == 0)
        return PER_SCOSVR4;
    if (strcmp(p, "wysev386") == 0)
        return PER_WYSEV386;
    if (strcmp(p, "iscr4") == 0)
        return PER_ISCR4;

    /* unknown personalities map to default Linux */
    return PER_LINUX;
}

const char *personality_to_string(unsigned long p) {
    switch (p) {
        case PER_LINUX: return "linux";
        case PER_LINUX_32BIT: return "linux32";
        case PER_SVR4: return "svr4";
        case PER_SCOSVR4: return "scosvr4";
        case PER_WYSEV386: return "wysev386";
        case PER_ISCR4: return "iscr4";
        default: return "linux"; /* fallback */
    }
}

/* macOS: no OOM killer, always accept values between -1000 and 1000 for compatibility */
bool oom_score_adjust_is_valid(int oa) {
        return oa >= -1000 && oa <= 1000;
}

static void reset_signals_to_default(void) {
        for (int sig = 1; sig < NSIG; sig++) {
                if (sig == SIGKILL || sig == SIGSTOP)
                        continue;
                signal(sig, SIG_DFL);
        }
}

static void clear_cloexec_all(void) {
        int max_fd = (int) sysconf(_SC_OPEN_MAX);
        if (max_fd < 0)
                max_fd = 1024;

        for (int fd = 0; fd < max_fd; fd++) {
                int flags = fcntl(fd, F_GETFD);
                if (flags < 0)
                        continue;

                flags &= ~FD_CLOEXEC;
                (void) fcntl(fd, F_SETFD, flags);
        }
}

int namespace_fork(
                const char *outer_name,
                const char *inner_name,
                const int except_fds[],
                size_t n_except_fds,
                ForkFlags flags,
                int pidns_fd,
                int mntns_fd,
                int netns_fd,
                int userns_fd,
                int root_fd,
                pid_t *ret_pid) {

        pid_t pid;

        /* Linux-only features — ignored on macOS */
        (void) pidns_fd;
        (void) mntns_fd;
        (void) netns_fd;
        (void) userns_fd;
        (void) root_fd;
        (void) outer_name;

        pid = fork();
        if (pid < 0)
                return -errno;

        if (pid > 0) {
                if (ret_pid)
                        *ret_pid = pid;
                return 0;
        }

        /* ===== Child ===== */
        if (inner_name)
                setprogname(inner_name);

        if (flags & FORK_RESET_SIGNALS)
                reset_signals_to_default();

        if (flags & FORK_CLOSE_ALL_FDS) {
                int max_fd = (int) sysconf(_SC_OPEN_MAX);
                if (max_fd < 0)
                        max_fd = 1024;

                for (int fd = 3; fd < max_fd; fd++) {
                        bool keep = false;

                        for (size_t i = 0; i < n_except_fds; i++) {
                                if (fd == except_fds[i]) {
                                        keep = true;
                                        break;
                                }
                        }

                        if (!keep)
                                (void) close(fd);
                }
        }

        if (flags & FORK_CLOEXEC_OFF)
                clear_cloexec_all();

        return 0;
}

bool invoked_by_systemd(void) {
    return false;
}

int is_kernel_thread(pid_t pid) {
    (void)pid; // suppress unused parameter warning
    return 0;  // macOS has no kernel threads visible to user space
}

pid_t getpid_cached(void) {
    static _Atomic pid_t cached_pid = 0;
    pid_t pid = atomic_load(&cached_pid);

    if (pid == 0) {
        pid = getpid();
        atomic_store(&cached_pid, pid);
    }

    return pid;
}

_noreturn_ void freeze(void) {
    fprintf(stderr, "freeze() not implemented on macOS\n");
    abort(); // or exit(ENOSYS)
}

int getenv_for_pid(pid_t pid, const char *field, char **_value) {
    char path[PATH_MAX];
    snprintf(path, sizeof(path), "/proc/%d/environ", pid);

    FILE *f = fopen(path, "r");
    if (!f)
        return -errno;

    char *buf = NULL;
    size_t len = 0;
    ssize_t r = getdelim(&buf, &len, '\0', f);
    fclose(f);

    if (r < 0)
        return -errno;

    for (char *p = buf; p < buf + r; p += strlen(p)+1) {
        if (strncmp(p, field, strlen(field)) == 0 && p[strlen(field)] == '=') {
            *_value = strdup(p + strlen(field) + 1);
            free(buf);
            return 0;
        }
    }

    free(buf);
    return -ENOENT;
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

int wait_for_terminate_and_check(const char *name, pid_t pid, WaitFlags flags) {
    int status;
    pid_t ret;
    int options = 0;

    if (flags & WAIT_NOHANG)
        options |= WNOHANG;
    if (flags & WAIT_UNTRACED)
        options |= WUNTRACED;

    do {
        ret = waitpid(pid, &status, options);
    } while (ret == -1 && errno == EINTR);

    if (ret == -1) {
        perror("waitpid failed");
        return -errno;
    }

    if (WIFEXITED(status)) {
        int exit_status = WEXITSTATUS(status);
        if (exit_status != 0) {
            fprintf(stderr, "%s (pid %d) exited with status %d\n", name, pid, exit_status);
            return exit_status;
        }
        return 0;
    } else if (WIFSIGNALED(status)) {
        int sig = WTERMSIG(status);
        fprintf(stderr, "%s (pid %d) killed by signal %d\n", name, pid, sig);
        return -sig;
    }

    return 0; // shouldn't reach here usually
}
