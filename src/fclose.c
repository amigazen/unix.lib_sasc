/*
 * SPDX-License-Identifier: BSD-2-Clause
 * fclose.c - Close file stream (POSIX compliant)
 *
 * This function closes a file stream and flushes any pending output.
 * It also frees any dynamically allocated buffers.
 *
 * POSIX.1-2001, POSIX.1-2008, C89, C99
 * Copyright (C) 2025 by amigazen project
 */

#ifndef _WITH_SCLIB

#include "amiga.h"
#include <stdio.h>
#include <errno.h>

/* Forward declarations */
extern int _doflush(FILE *fp, int c);
extern int close(int fd);
extern void free(void *ptr);

/**
 * @brief Close file stream
 * @param fp File stream to close
 * @return 0 on success, EOF on failure
 * 
 * The fclose() function causes the stream pointed to by fp to be flushed
 * and the associated file to be closed. Any unwritten buffered data for
 * the stream are delivered to the host environment to be written to the
 * file; any unread buffered data are discarded.
 * 
 * This implementation:
 * - Flushes any pending output before closing
 * - Frees dynamically allocated buffers
 * - Properly resets stream state
 * - Uses Amiga-optimized file operations
 * - Maintains C89 compliance for SAS/C compiler
 */
int fclose(FILE *fp)
{
    int result;
    
    /* Validate parameter */
    if (fp == NULL) {
        errno = EINVAL;
        return EOF;
    }
    
    result = 0;
    
    /* Only process if file is active */
    if (fp->_file >= 0) {
        /* Flush any pending output */
        if (fp->_flag & _IOWRT) {
            result = _doflush(fp, -1);
        }
        
        /* Close the file descriptor */
        result |= close((int)fp->_file);
        
        /* Free dynamically allocated buffer if needed */
        if (fp->_base && (fp->_flag & _IOMYBUF)) {
            free(fp->_base);
        }
    }
    
    /* Reset stream state */
    fp->_base = NULL;
    fp->_ptr = NULL;
    fp->_rcnt = 0;
    fp->_wcnt = 0;
    fp->_file = -1;
    fp->_flag = 0;
    
    return result;
}

/**
 * @brief Flush file stream
 * @param fp File stream to flush
 * @return 0 on success, EOF on failure
 * 
 * The fflush() function causes any unwritten data for the stream to be
 * delivered to the host environment to be written to the file.
 * 
 * This implementation:
 * - Uses the internal _doflush() function
 * - Handles both specific streams and all streams (NULL)
 * - Maintains POSIX compliance
 */
int fflush(FILE *fp)
{
    return _doflush(fp, -1);
}

#else
/* Empty - functions provided by SAS/C sc.lib */
#endif
