#ifndef LINUX_FS_H_PORT
#define LINUX_FS_H_PORT

#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/disk.h>

#define FS_IOC_GETFLAGS _IOR('f', 1, long)
#define FS_IOC_SETFLAGS _IOW('f', 2, long)

#define FS_NOATIME_FL 0
#define FS_COMPR_FL   0
#define FS_NOCOMP_FL  0

/* --------------------------
 * File type constants
 * -------------------------- */
#ifndef S_IFMT
#define S_IFMT 0170000
#endif
#ifndef S_IFSOCK
#define S_IFSOCK 0140000
#endif
#ifndef S_IFLNK
#define S_IFLNK 0120000
#endif
#ifndef S_IFREG
#define S_IFREG 0100000
#endif
#ifndef S_IFBLK
#define S_IFBLK 0060000
#endif
#ifndef S_IFDIR
#define S_IFDIR 0040000
#endif
#ifndef S_IFCHR
#define S_IFCHR 0020000
#endif
#ifndef S_IFIFO
#define S_IFIFO 0010000
#endif

/* --------------------------
 * File type macros
 * -------------------------- */
#ifndef S_ISREG
#define S_ISREG(m)  (((m) & S_IFMT) == S_IFREG)
#endif
#ifndef S_ISDIR
#define S_ISDIR(m)  (((m) & S_IFMT) == S_IFDIR)
#endif
#ifndef S_ISCHR
#define S_ISCHR(m)  (((m) & S_IFMT) == S_IFCHR)
#endif
#ifndef S_ISBLK
#define S_ISBLK(m)  (((m) & S_IFMT) == S_IFBLK)
#endif
#ifndef S_ISFIFO
#define S_ISFIFO(m) (((m) & S_IFMT) == S_IFIFO)
#endif
#ifndef S_ISLNK
#define S_ISLNK(m)  (((m) & S_IFMT) == S_IFLNK)
#endif
#ifndef S_ISSOCK
#define S_ISSOCK(m) (((m) & S_IFMT) == S_IFSOCK)
#endif

/* macOS / BSD: chattr not supported */
#define FS_SECRM_FL   0
#define FS_NODUMP_FL  0
#define FS_SYNC_FL    0
#define FS_NOCOW_FL   0

/* --------------------------
 * Linux block device ioctl mappings
 * -------------------------- */
/* Linux      -> macOS equivalent */
#define BLKGETSIZE      0x00001260 /* bytes? Linux: sectors -> macOS: use DKIOCGETBLOCKCOUNT */
#define BLKSSZGET       0x00001268 /* sector size -> DKIOCGETBLOCKSIZE */

static inline int ioctl_blkgetsize(int fd, uint64_t *size) {
    uint64_t blocks;
    if (ioctl(fd, DKIOCGETBLOCKCOUNT, &blocks) < 0) return -1;
    *size = blocks;
    return 0;
}

static inline int ioctl_blksszget(int fd, uint32_t *blksize) {
    uint32_t bs;
    if (ioctl(fd, DKIOCGETBLOCKSIZE, &bs) < 0) return -1;
    *blksize = bs;
    return 0;
}

/* --------------------------
 * Kernel-only struct stubs
 * -------------------------- */
struct file {
    int fd;
};

struct inode {
    uint32_t i_mode;
    uint64_t i_ino;
};

struct super_block {
    uint32_t s_blocksize;
    uint64_t s_blocks;
};

/* --------------------------
 * Useful helpers
 * -------------------------- */
static inline bool is_regular_file(int fd) {
    struct stat st;
    if (fstat(fd, &st) < 0) return false;
    return S_ISREG(st.st_mode);
}

static inline bool is_directory(int fd) {
    struct stat st;
    if (fstat(fd, &st) < 0) return false;
    return S_ISDIR(st.st_mode);
}

#endif /* LINUX_FS_H_PORT */
