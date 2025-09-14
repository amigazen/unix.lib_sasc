/*
 * SPDX-License-Identifier: BSD-2-Clause
 * fseek.c - File positioning functions
 *
 * Based on PDC fseek.c by J.A. Lydiatt
 * Copyright (C) 2025 by amigazen project
 */

#ifndef _WITH_SCLIB

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "amiga.h"

/* Forward declarations */
extern int _doflush(FILE *fp, int c);

/**
 * @brief Set file position indicator
 * @param fp FILE stream
 * @param offset Offset from origin
 * @param origin Origin position (SEEK_SET, SEEK_CUR, SEEK_END)
 * @return 0 on success, -1 on error
 * 
 * Sets the file position indicator for the stream.
 * This is essential for random access file operations.
 */
int fseek(FILE *fp, long offset, int origin)
{
    long newpos;
    
    if (fp == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    /* Clear EOF flag */
    fp->_flag &= ~_IOEOF;
    
    /* Flush buffer if writing */
    if (fp->_flag & _IOWRT) {
        if (_doflush(fp, -1) == EOF) {
            return -1;
        }
    } else if (origin == SEEK_CUR && fp->_ptr != NULL) {
        /* Adjust offset for current buffer position */
        offset -= (long)(fp->_rcnt - (fp->_ptr - fp->_base));
    }
    
    /* Clear buffer pointers */
    fp->_ptr = NULL;
    fp->_rcnt = 0;
    fp->_wcnt = 0;
    
    /* Perform the seek */
    newpos = lseek(fp->_file, offset, origin);
    if (newpos < 0) {
        return -1;
    }
    
    return 0;
}

/**
 * @brief Get current file position
 * @param fp FILE stream
 * @return Current position or -1 on error
 * 
 * Returns the current file position indicator.
 */
long ftell(FILE *fp)
{
    long position;
    
    if (fp == NULL) {
        errno = EINVAL;
        return -1L;
    }
    
    /* Get current position from file descriptor */
    position = lseek(fp->_file, 0L, SEEK_CUR);
    if (position < 0) {
        return -1L;
    }
    
    /* Adjust for buffer position */
    if (fp->_flag & _IOWRT) {
        /* Writing: add characters written to buffer */
        position += (long)(fp->_ptr - fp->_base);
    } else if (fp->_ptr != NULL) {
        /* Reading: subtract characters remaining in buffer */
        position -= (long)(fp->_rcnt - (fp->_ptr - fp->_base));
    }
    
    return position;
}

/* Note: rewind() is provided by SAS/C stdio.h as a macro that calls fseek(fp, 0L, 0) */

#else
/* Empty - functions provided by SAS/C sc.lib */
#endif
