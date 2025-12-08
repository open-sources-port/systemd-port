#pragma once

#include <unistd.h>
#include <copyfile.h>
#include <errno.h>

/* Partial copy_file_range using fcopyfile (works only on regular files) */
static inline ssize_t copy_file_range(int fd_in, off_t *off_in,
                                      int fd_out, off_t *off_out,
                                      size_t len, unsigned int flags)
{
    (void)flags;

    if (off_in) lseek(fd_in, *off_in, SEEK_SET);
    if (off_out) lseek(fd_out, *off_out, SEEK_SET);

    if (fcopyfile(fd_in, fd_out, NULL, COPYFILE_DATA) != 0) {
        return -1;
    }

    ssize_t copied = (ssize_t) len; // best-effort, may not match len exactly
    if (off_in) *off_in += copied;
    if (off_out) *off_out += copied;

    return copied;
}
