/*
 * SPDX-License-Identifier: BSD-2-Clause
 * strchr.c - locate character in string (POSIX compliant)
 *
 * This function returns a pointer to the first occurrence of the character 'c'
 * in the string 's'. The terminating null byte is considered part of the string,
 * so that if 'c' is specified as '\0', the function returns a pointer to the
 * terminator.
 *
 * POSIX.1-2001, POSIX.1-2008, C89, C99
 * Copyright (C) 2025 by amigazen project
 */

#include <string.h>

/**
 * @brief Locate character in string
 * @param s Pointer to the string to search
 * @param c Character to search for
 * @return Pointer to the first occurrence of c in s, or NULL if c is not found
 * 
 * The strchr() function returns a pointer to the first occurrence of the character
 * 'c' in the string 's'. The terminating null byte is considered part of the string,
 * so that if 'c' is specified as '\0', the function returns a pointer to the
 * terminator.
 * 
 * This implementation:
 * - Searches for the first occurrence of c in the string s
 * - Returns pointer to the character if found
 * - Returns NULL if c is not found
 * - Can find the null terminator if c is '\0'
 * - Is optimized for performance with register variables
 * - Is C89 compliant for SAS/C compiler compatibility
 */
char *strchr(const char *s, int c)
{
    const char *scan;
    
    /* Search through the string */
    for (scan = s; *scan != c; scan++) {
        if (*scan == '\0') {
            /* Reached end of string without finding c */
            return NULL;
        }
    }
    
    /* Found the character */
    return (char *)scan;
}
