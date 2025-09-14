/*
 * SPDX-License-Identifier: BSD-2-Clause
 * fputc.c - Put character to file stream (POSIX compliant)
 *
 * This function writes a character to a file stream with buffering
 * for improved performance and automatic flushing for TTY streams.
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

/**
 * @brief Put character to file stream
 * @param c Character to write (converted to unsigned char)
 * @param stream File stream to write to
 * @return Character written on success, EOF on failure
 * 
 * The fputc() function writes the character c (converted to an unsigned
 * char) to the output stream pointed to by stream, at the position
 * indicated by the associated file position indicator for the stream
 * (if defined), and advances the indicator appropriately.
 * 
 * This implementation:
 * - Uses buffered I/O for performance
 * - Handles buffer flushing automatically
 * - Flushes TTY streams on newline/carriage return
 * - Maintains C89 compliance for SAS/C compiler
 * - Uses internal _doflush() for buffer management
 */
int fputc(int c, FILE *stream)
{
    /* Validate parameter */
    if (stream == NULL) {
        errno = EINVAL;
        return EOF;
    }
    
    /* Check if buffer needs flushing */
    if (stream->_wcnt <= 0) {
        return _doflush(stream, c);
    }
    
    /* Write character to buffer */
    *stream->_ptr++ = (unsigned char)c;
    stream->_wcnt--;
    
    /* Flush TTY streams on newline or carriage return */
    if ((stream->_flag & _IOLBF) && (c == '\n' || c == '\r')) {
        int result = _doflush(stream, c);
        --stream->_ptr;  /* Adjust position after flush */
        return result;
    }
    
    return c;
}

#else
/* Empty - functions provided by SAS/C sc.lib */
#endif
