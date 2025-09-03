/*
 * SPDX-License-Identifier: BSD-2-Clause
 * strcpy.c - copy a string (POSIX compliant)
 *
 * This function copies the string pointed to by 'src', including the terminating
 * null byte ('\0'), to the buffer pointed to by 'dest'.
 *
 * POSIX.1-2001, POSIX.1-2008, C89, C99
 * Copyright (C) 2025 by amigazen project
 */

#include <string.h>

/**
 * @brief Copy a string
 * @param dest Pointer to the destination buffer
 * @param src Pointer to the source string
 * @return Pointer to the destination string (dest)
 * 
 * The strcpy() function copies the string pointed to by 'src', including the
 * terminating null byte ('\0'), to the buffer pointed to by 'dest'.
 * 
 * This implementation:
 * - Copies the entire source string including the null terminator
 * - Returns a pointer to the destination string
 * - Does not perform bounds checking (caller must ensure sufficient space)
 * - Is optimized for performance with register variables
 * - Is C89 compliant for SAS/C compiler compatibility
 * 
 * Warning: The destination buffer must be large enough to hold the source string
 * plus the terminating null character. No bounds checking is performed.
 */
char *strcpy(char *dest, const char *src)
{
    char *dscan;
    const char *sscan;
    
    dscan = dest;
    sscan = src;
    
    /* Copy characters until we reach the null terminator */
    while ((*dscan++ = *sscan++) != '\0') {
        /* Empty loop body - assignment and increment happen in condition */
    }
    
    return dest;
}
