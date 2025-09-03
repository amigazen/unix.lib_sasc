/*
 * sys/statvfs.h - POSIX filesystem statistics
 *
 * Copyright (C) 2025 by amigazen project
 */

#ifndef _SYS_STATVFS_H
#define _SYS_STATVFS_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

/* Define filesystem types if not already defined */
#ifndef _FSBLKCNT_T
#define _FSBLKCNT_T 1
typedef unsigned long fsblkcnt_t;  /* Used for filesystem block counts */
#endif

#ifndef _FSFILCNT_T
#define _FSFILCNT_T 1
typedef unsigned long fsfilcnt_t;  /* Used for filesystem file counts */
#endif

/*
 * POSIX statvfs structure - Filesystem statistics
 * Based on POSIX.1-2001 standard
 */
struct statvfs {
    unsigned long f_bsize;    /* Filesystem block size */
    unsigned long f_frsize;   /* Fragment size */
    fsblkcnt_t   f_blocks;   /* Total number of blocks */
    fsblkcnt_t   f_bfree;    /* Number of free blocks */
    fsblkcnt_t   f_bavail;   /* Number of available blocks for non-root */
    fsfilcnt_t   f_files;    /* Total number of file nodes */
    fsfilcnt_t   f_ffree;    /* Number of free file nodes */
    fsfilcnt_t   f_favail;   /* Number of available file nodes for non-root */
    unsigned long f_fsid;     /* Filesystem ID */
    unsigned long f_flag;     /* Mount flags */
    unsigned long f_namemax;  /* Maximum filename length */
    
    /* Amiga-specific extensions */
    char         f_fstypename[32]; /* Filesystem type name (OFS, FFS, etc.) */
    char         f_mntonname[256]; /* Mount point name */
    char         f_mntfromname[256]; /* Device name */
};

/*
 * Mount flags
 */
#define ST_RDONLY      0x0001  /* Read-only filesystem */
#define ST_NOSUID      0x0002  /* No set-user-id bit */
#define ST_NODEV       0x0004  /* No device files */
#define ST_NOEXEC      0x0008  /* No executable files */
#define ST_SYNCHRONOUS 0x0010  /* Synchronous writes */
#define ST_MANDLOCK    0x0040  /* Mandatory locking */
#define ST_WRITE       0x0080  /* Write access */
#define ST_APPEND      0x0100  /* Append-only */
#define ST_IMMUTABLE   0x0200  /* Immutable */
#define ST_NOATIME     0x0400  /* No access time updates */
#define ST_NODIRATIME  0x0800  /* No directory access time updates */
#define ST_RELATIME    0x1000  /* Relative access time updates */

/*
 * Function prototypes
 */

/* statvfs() - Get filesystem statistics by path */
int statvfs(const char *path, struct statvfs *buf);

/* fstatvfs() - Get filesystem statistics by file descriptor */
int fstatvfs(int fd, struct statvfs *buf);

#ifdef __cplusplus
}
#endif

#endif /* _SYS_STATVFS_H */
