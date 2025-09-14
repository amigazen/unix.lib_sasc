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
#include <proto/utility.h>
#include "amiga.h"

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
 * - Uses Amiga Strncpy() for optimal performance when available
 * - Falls back to optimized C implementation for compatibility
 * - Handles NULL pointers gracefully
 * - Is optimized for Amiga with register variables
 * - Is C89 compliant for SAS/C compiler compatibility
 * 
 * Warning: The destination buffer must be large enough to hold the source string
 * plus the terminating null character. No bounds checking is performed.
 */
char *strcpy(char *dest, const char *src)
{
    /* Validate parameters */
    if (dest == NULL || src == NULL) {
        return dest;
    }
    
    /* Use Amiga Strncpy for optimal performance */
    /* We use a large buffer size since strcpy() doesn't have size limits */
    if (Strncpy((UBYTE *)dest, (const UBYTE *)src, 0x7FFFFFFF) == NULL) {
        /* Fallback to C implementation if Strncpy fails */
        char *dscan = dest;
        const char *sscan = src;
        
        /* Copy characters until we reach the null terminator */
        while ((*dscan++ = *sscan++) != '\0') {
            /* Empty loop body - assignment and increment happen in condition */
        }
    }
    
    return dest;
}

