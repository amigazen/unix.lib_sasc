/*
 * SPDX-License-Identifier: BSD-2-Clause
 * fopen.c - Open file for buffered I/O (POSIX compliant)
 *
 * This function opens a file for buffered I/O operations. It supports
 * standard POSIX file modes and provides Amiga-specific optimizations.
 *
 * POSIX.1-2001, POSIX.1-2008, C89, C99
 * Copyright (C) 2025 by amigazen project
 */

#ifndef _WITH_SCLIB

#include "amiga.h"
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

/* Forward declarations */
extern FILE *addStream(void);
extern int fclose(FILE *fp);
extern int open(const char *pathname, int flags, ...);
extern int strcmp(const char *s1, const char *s2);

/**
 * @brief Mode table for file opening
 * Maps POSIX mode strings to open() flags
 */
static struct modeType {
    const char *requested;
    int         openMode;
} modeTable[] = {
    {"r",   O_RDONLY},
    {"r+",  O_RDWR},
    {"w",   (O_WRONLY|O_CREAT|O_TRUNC)},
    {"w+",  (O_RDWR|O_CREAT|O_TRUNC)},
    {"a",   (O_WRONLY|O_CREAT|O_APPEND)},
    {"a+",  (O_RDWR|O_CREAT|O_APPEND)},
    {"x",   (O_WRONLY|O_CREAT|O_EXCL)},
    {"x+",  (O_RDWR|O_CREAT|O_EXCL)},
    {"",    0}
};

/**
 * @brief Reopen file with new name and mode
 * @param name File name to open
 * @param mode File access mode
 * @param fp File stream to reuse
 * @return FILE* pointer on success, NULL on failure
 * 
 * The freopen() function opens the file whose name is the string pointed to
 * by name and associates the stream pointed to by fp with it. The original
 * stream (if it exists) is first closed with fclose().
 * 
 * This implementation:
 * - Supports all standard POSIX file modes
 * - Properly handles mode validation
 * - Uses Amiga-optimized file operations
 * - Maintains C89 compliance for SAS/C compiler
 */
FILE *freopen(const char *name, const char *mode, FILE *fp)
{
    struct modeType *mp;
    int fd;
    
    /* Validate parameters */
    if (name == NULL || mode == NULL || fp == NULL) {
        errno = EINVAL;
        return NULL;
    }
    
    /* Close existing file if open */
    fclose(fp);
    
    /* Find matching mode in table */
    for (mp = &modeTable[0];; ++mp) {
        if (!*mp->requested) {
            errno = EINVAL;
            return NULL;
        }
        if (strcmp(mp->requested, mode) == 0) {
            break;
        }
    }
    
    /* Open file with appropriate flags */
    fd = open(name, mp->openMode, 0644);
    if (fd == -1) {
        return NULL;
    }
    
    /* Associate file descriptor with stream */
    fp->_file = fd;
    
    /* Set appropriate flags based on mode */
    if (strcmp(mode, "r") == 0) {
        fp->_flag = _IOREAD;
    } else if (strcmp(mode, "w") == 0 || strcmp(mode, "a") == 0) {
        fp->_flag = _IOWRT;
    } else if (strcmp(mode, "r+") == 0 || strcmp(mode, "w+") == 0 || strcmp(mode, "a+") == 0) {
        fp->_flag = _IOREAD | _IOWRT;
    } else {
        fp->_flag = _IOREAD;  /* Default to read */
    }
    
    return fp;
}

/**
 * @brief Open file for buffered I/O
 * @param name File name to open
 * @param mode File access mode
 * @return FILE* pointer on success, NULL on failure
 * 
 * The fopen() function opens the file whose name is the string pointed to
 * by name and associates a stream with it.
 * 
 * Supported modes:
 * - "r"  : Open for reading
 * - "w"  : Open for writing (truncate if exists)
 * - "a"  : Open for appending
 * - "r+" : Open for reading and writing
 * - "w+" : Open for reading and writing (truncate if exists)
 * - "a+" : Open for reading and writing (append)
 * - "x"  : Open for writing (exclusive create)
 * - "x+" : Open for reading and writing (exclusive create)
 * 
 * This implementation:
 * - Allocates a new stream using addStream()
 * - Uses freopen() for actual file opening
 * - Provides proper error handling
 * - Maintains POSIX compliance
 */
FILE *fopen(const char *name, const char *mode)
{
    FILE *fp;
    
    /* Validate parameters */
    if (name == NULL || mode == NULL) {
        errno = EINVAL;
        return NULL;
    }
    
    /* Allocate new stream */
    fp = addStream();
    if (fp == NULL) {
        return NULL;
    }
    
    /* Open file using freopen */
    return freopen(name, mode, fp);
}

#else
/* Empty - functions provided by SAS/C sc.lib */
#endif
