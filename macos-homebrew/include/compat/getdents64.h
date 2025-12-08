#pragma once

#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>

/* Linux-like dirent64 struct */
struct linux_dirent64 {
    uint64_t d_ino;        // inode number
    int64_t  d_off;        // offset to next dirent
    unsigned short d_reclen; // length of this dirent
    unsigned char  d_type;   // file type
    char           d_name[256]; // filename
};

/* Map macOS DT_* to Linux d_type */
static inline unsigned char convert_dtype(unsigned char dt) {
    switch (dt) {
        case DT_REG:  return 8;   // DT_REG on Linux
        case DT_DIR:  return 4;   // DT_DIR on Linux
        case DT_LNK:  return 10;  // DT_LNK on Linux
        case DT_FIFO: return 1;   // DT_FIFO on Linux
        case DT_SOCK: return 12;  // DT_SOCK on Linux
        case DT_CHR:  return 2;   // DT_CHR on Linux
        case DT_BLK:  return 6;   // DT_BLK on Linux
        default:      return 0;
    }
}

/* MacOS getdents64 emulation */
static inline int getdents64(int fd, struct linux_dirent64 *dirp, unsigned int count) {
    if (!dirp || count == 0) {
        errno = EINVAL;
        return -1;
    }

    DIR *dp = fdopendir(fd);
    if (!dp) {
        return -1; // errno set by fdopendir
    }

    int bytes_written = 0;
    struct dirent *entry;

    while ((entry = readdir(dp)) != NULL) {
        struct linux_dirent64 d;
        memset(&d, 0, sizeof(d));

        d.d_ino = entry->d_ino;
        d.d_off = telldir(dp);            // pseudo-offset
        d.d_type = convert_dtype(entry->d_type);
        strncpy(d.d_name, entry->d_name, sizeof(d.d_name) - 1);
        d.d_reclen = sizeof(d);

        if (bytes_written + (int)d.d_reclen > (int)count) {
            break; // no more space in user buffer
        }

        memcpy((char*)dirp + bytes_written, &d, d.d_reclen);
        bytes_written += d.d_reclen;
    }

    /* Rewind the DIR so fd remains usable for next call */
    rewinddir(dp);
    /* Do NOT closedir(dp) — fd remains open for repeated calls */

    return bytes_written;
}
