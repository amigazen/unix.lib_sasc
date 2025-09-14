/*
 * SPDX-License-Identifier: BSD-2-Clause
 * fputs.c - Put string to file stream (POSIX compliant)
 *
 * This function writes a null-terminated string to a file stream.
 * It does not write the null terminator itself.
 *
 * POSIX.1-2001, POSIX.1-2008, C89, C99
 * Copyright (C) 2025 by amigazen project
 */

#ifndef _WITH_SCLIB

#include "amiga.h"
#include <stdio.h>
#include <errno.h>

/* Forward declarations */
extern int fputc(int c, FILE *stream);

/**
 * @brief Put string to file stream
 * @param s Null-terminated string to write
 * @param stream File stream to write to
 * @return Non-negative value on success, EOF on failure
 * 
 * The fputs() function writes the string pointed to by s to the stream
 * pointed to by stream. The terminating null character is not written.
 * 
 * This implementation:
 * - Writes each character using fputc()
 * - Stops at null terminator
 * - Returns EOF on any write error
 * - Returns number of characters written on success
 * - Maintains C89 compliance for SAS/C compiler
 */
int fputs(const char *s, FILE *stream)
{
    int c;
    int count = 0;
    
    /* Validate parameters */
    if (s == NULL || stream == NULL) {
        errno = EINVAL;
        return EOF;
    }
    
    /* Write each character until null terminator */
    while ((c = *s++) != '\0') {
        if (fputc(c, stream) == EOF) {
            return EOF;
        }
        count++;
    }
    
    return count;
}

#else
/* Empty - functions provided by SAS/C sc.lib */
#endif
