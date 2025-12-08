// linux_magic_compat.h
#pragma once

// Define Linux magic values if missing
#ifndef TMPFS_MAGIC
#define TMPFS_MAGIC        0x01021994
#endif

#ifndef BTRFS_SUPER_MAGIC
#define BTRFS_SUPER_MAGIC  0x9123683E
#endif

#ifndef EXT4_SUPER_MAGIC
#define EXT4_SUPER_MAGIC   0xEF53
#endif

#ifndef XFS_SUPER_MAGIC
#define XFS_SUPER_MAGIC    0x58465342
#endif

// Add more depending on what systemd header requires
