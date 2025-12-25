#!/usr/bin/env python3
# SPDX-License-Identifier: LGPL-2.1-or-later

import sys

def filter_fsname(name):
    return name in {
        "cpuset", "devtmpfs", "ext2", "ext3", "fuseblk", "gfs",
        "msdos", "ncp", "nfs", "pvfs2", "smb3",
    }

gperf_file = sys.argv[1]
keywords_section = False
magic_to_name = {}  # map magic number to filesystem name

for line in open(gperf_file):
    if line[0] == "#":
        continue

    if keywords_section:
        name, ids = line.split(",", 1)
        name = name.strip()
        if filter_fsname(name):
            continue

        ids = ids.strip()
        assert ids[0] == "{"
        assert ids[-1] == "}"
        ids = ids[1:-1]

        for id in ids.split(","):
            id = id.strip()
            # always overwrite previous mapping to prefer the newer FS
            magic_to_name[id] = name

    if line.startswith("%%"):
        keywords_section = True

# generate switch-case statements
for magic, name in magic_to_name.items():
    print(f"case (statfs_f_type_t) {magic}:")
    print(f'        return "{name}";')
