/*
 * SPDX-License-Identifier: BSD-2-Clause
 * fgetc.c - Get character from file stream (POSIX compliant)
 *
 * This function reads a character from a file stream with buffering
 * for improved performance.
 *
 * POSIX.1-2001, POSIX.1-2008, C89, C99
 * Copyright (C) 2025 by amigazen project
 */

#ifndef _WITH_SCLIB

#include "amiga.h"
#include <stdio.h>
#include <errno.h>

/* Forward declarations */
extern int _filebfill(FILE *fp);

/**
 * @brief Get character from file stream
 * @param stream File stream to read from
 * @return Character read on success, EOF on failure or end of file
 * 
 * The fgetc() function obtains the next character (if present) as an
 * unsigned char converted to an int, from the input stream pointed to
 * by stream, and advances the associated file position indicator for
 * the stream (if defined).
 * 
 * This implementation:
 * - Uses buffered I/O for performance
 * - Handles buffer refilling automatically
 * - Returns EOF on error or end of file
 * - Maintains C89 compliance for SAS/C compiler
 * - Uses internal _filebfill() for buffer management
 */
int fgetc(FILE *stream)
{
    /* Validate parameter */
    if (stream == NULL) {
        errno = EINVAL;
        return EOF;
    }
    
    /* Check if buffer needs refilling */
    if (stream->_rcnt <= 0) {
        return _filebfill(stream);
    }
    
    /* Return next character from buffer */
    stream->_rcnt--;
    return (*stream->_ptr++ & 0xFF);
}

/**
 * @brief Push character back to file stream
 * @param c Character to push back
 * @param stream File stream to push back to
 * @return Character pushed back on success, EOF on failure
 * 
 * The ungetc() function pushes the character c (converted to an unsigned
 * char) back onto the input stream pointed to by stream. Pushed-back
 * characters will be returned by subsequent reads on that stream in the
 * reverse order of their pushing.
 * 
 * This implementation:
 * - Validates character and stream parameters
 * - Checks for buffer space availability
 * - Maintains C89 compliance for SAS/C compiler
 */
int ungetc(int c, FILE *stream)
{
    /* Validate parameters */
    if (c == EOF || stream == NULL) {
        return EOF;
    }
    
    /* Check if there's space in buffer */
    if (stream->_ptr <= stream->_base) {
        return EOF;
    }
    
    /* Push character back into buffer */
    *--stream->_ptr = (unsigned char)c;
    stream->_rcnt++;
    return c;
}

#else
/* Empty - functions provided by SAS/C sc.lib */
#endif
