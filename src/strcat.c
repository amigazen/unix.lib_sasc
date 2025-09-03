/*
 * SPDX-License-Identifier: BSD-2-Clause
 * strcat.c - concatenate two strings (POSIX compliant)
 *
 * This function appends the string pointed to by 'src' to the end of the string
 * pointed to by 'dest'. The terminating null byte in 'dest' is overwritten by
 * the first character of 'src', and a null byte is included at the end of the
 * new string formed by the concatenation of both in 'dest'.
 *
 * POSIX.1-2001, POSIX.1-2008, C89, C99
 * Copyright (C) 2025 by amigazen project
 */

#include <string.h>

/**
 * @brief Concatenate two strings
 * @param dest Pointer to the destination string (must be null-terminated)
 * @param src Pointer to the source string to append
 * @return Pointer to the destination string (dest)
 * 
 * The strcat() function appends the string pointed to by 'src' to the end of
 * the string pointed to by 'dest'. The terminating null byte in 'dest' is
 * overwritten by the first character of 'src', and a null byte is included
 * at the end of the new string formed by the concatenation of both in 'dest'.
 * 
 * This implementation:
 * - Appends the source string to the end of the destination string
 * - Returns a pointer to the destination string
 * - Does not perform bounds checking (caller must ensure sufficient space)
 * - Is optimized for performance with register variables
 * - Is C89 compliant for SAS/C compiler compatibility
 * 
 * Warning: The destination buffer must be large enough to hold both strings
 * plus the terminating null character. No bounds checking is performed.
 */
char *strcat(char *dest, const char *src)
{
    char *dscan;
    const char *sscan;
    
    /* Find the end of the destination string */
    for (dscan = dest; *dscan != '\0'; dscan++) {
        /* Empty loop body - just advance to end of string */
    }
    
    /* Append the source string to the destination */
    sscan = src;
    while ((*dscan++ = *sscan++) != '\0') {
        /* Empty loop body - assignment and increment happen in condition */
    }
    
    return dest;
}
