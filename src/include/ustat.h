/*
 * ustat.h - Header file for ustat() function
 *
 * Copyright (C) 1995 by Ingo Wilken
 * Copyright (C) 2025 by amigazen project.
 */

#ifndef _USTAT_H
#define _USTAT_H 1

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ustat structure - Contains file system statistics
 */
struct ustat {
    long    f_tfree;        /* Total free blocks (Kbytes) */
    long    f_tinode;       /* Unix: Number of free inodes, Amiga: number of free (physical) blocks */
    char    f_fname[256];   /* Unix: Filesys name, Amiga: volume name */
    char    f_fpack[6];     /* unused */
};

/*
 * Function prototypes
 */

/* ustat() - Get file system statistics */
int ustat(long dev, struct ustat *ub);

#ifdef __cplusplus
}
#endif

#endif /* _USTAT_H */
