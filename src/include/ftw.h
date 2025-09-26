/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * ftw.h - File tree walk
 * 
 * This header provides file tree walking functions for Amiga
 * 
 * Copyright (C) 2025 by amigazen project
 */

#ifndef _FTW_H
#define _FTW_H 1

#include <sys/stat.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* File type flags for ftw() */
#define FTW_F       0   /* Regular file */
#define FTW_D       1   /* Directory */
#define FTW_DNR     2   /* Directory that cannot be read */
#define FTW_NS      3   /* File that cannot be stat()ed */
#define FTW_SL      4   /* Symbolic link */
#define FTW_DP      5   /* Directory (post-order) */
#define FTW_SLN     6   /* Symbolic link to non-existent file */

/* Flags for nftw() */
#define FTW_PHYS    0x01    /* Physical walk, don't follow symlinks */
#define FTW_MOUNT   0x02    /* Don't cross mount points */
#define FTW_DEPTH   0x04    /* Post-order traversal */
#define FTW_CHDIR   0x08    /* Change to directory before processing */

/* Function prototypes */
int ftw(const char *path, int (*fn)(const char *, const struct stat *, int), int ndirs);
int nftw(const char *path, int (*fn)(const char *, const struct stat *, int, struct FTW *), int fd_limit, int flags);

/* Structure for nftw() */
struct FTW {
    int base;       /* Offset of filename in pathname */
    int level;      /* Depth relative to start of walk */
};

#ifdef __cplusplus
}
#endif

#endif /* _FTW_H */
