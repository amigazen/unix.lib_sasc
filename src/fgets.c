/*
 * SPDX-License-Identifier: BSD-2-Clause
 * fgets.c - Get string from file stream (POSIX compliant)
 *
 * This function reads a line from a file stream into a buffer.
 * It stops at newline or end of file and null-terminates the string.
 *
 * POSIX.1-2001, POSIX.1-2008, C89, C99
 * Copyright (C) 2025 by amigazen project
 */

#ifndef _WITH_SCLIB

#include "amiga.h"
#include <stdio.h>
#include <errno.h>

/* Forward declarations */
extern int fgetc(FILE *stream);

/**
 * @brief Get string from file stream
 * @param s Buffer to store the string
 * @param n Maximum number of characters to read (including null terminator)
 * @param stream File stream to read from
 * @return Pointer to buffer on success, NULL on failure or end of file
 * 
 * The fgets() function reads at most one less than the number of characters
 * specified by n from the stream pointed to by stream into the array pointed
 * to by s. No additional characters are read after a newline character (which
 * is retained) or after end-of-file. A null character is written immediately
 * after the last character read into the array.
 * 
 * This implementation:
 * - Reads characters until newline, EOF, or buffer full
 * - Always null-terminates the string
 * - Retains the newline character in the string
 * - Returns NULL on error or if no characters read
 * - Maintains C89 compliance for SAS/C compiler
 */
char *fgets(char *s, int n, FILE *stream)
{
    char *p;
    int c;
    
    /* Validate parameters */
    if (s == NULL || stream == NULL || n <= 0) {
        errno = EINVAL;
        return NULL;
    }
    
    p = s;
    
    /* Read characters until newline, EOF, or buffer full */
    while (--n > 0 && (c = fgetc(stream)) != EOF) {
        *p++ = c;
        if (c == '\n') {
            break;
        }
    }
    
    /* Null-terminate the string */
    *p = '\0';
    
    /* Return NULL if no characters were read and EOF was encountered */
    if (c == EOF && p == s) {
        return NULL;
    }
    
    return s;
}

#else
/* Empty - functions provided by SAS/C sc.lib */
#endif
