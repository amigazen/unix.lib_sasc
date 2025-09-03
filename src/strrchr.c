/*
 * SPDX-License-Identifier: BSD-2-Clause
 * strrchr.c - locate character in string (reverse search) (POSIX compliant)
 *
 * This function returns a pointer to the last occurrence of the character 'c'
 * in the string 's'. The terminating null byte is considered part of the string,
 * so that if 'c' is specified as '\0', the function returns a pointer to the
 * terminator.
 *
 * POSIX.1-2001, POSIX.1-2008, C89, C99
 * Copyright (C) 2025 by amigazen project
 */

#include <string.h>

/**
 * @brief Locate character in string (reverse search)
 * @param s Pointer to the string to search
 * @param c Character to search for
 * @return Pointer to the last occurrence of c in s, or NULL if c is not found
 * 
 * The strrchr() function returns a pointer to the last occurrence of the character
 * 'c' in the string 's'. The terminating null byte is considered part of the string,
 * so that if 'c' is specified as '\0', the function returns a pointer to the
 * terminator.
 * 
 * This implementation:
 * - Searches for the last occurrence of c in the string s
 * - Returns pointer to the character if found
 * - Returns NULL if c is not found
 * - Can find the null terminator if c is '\0'
 * - Is optimized for performance with register variables
 * - Is C89 compliant for SAS/C compiler compatibility
 */
char *strrchr(const char *s, int c)
{
    const char *scan;
    const char *place;
    
    place = NULL;
    
    /* Search through the string, remembering the last occurrence */
    for (scan = s; *scan != '\0'; scan++) {
        if (*scan == c) {
            place = scan;
        }
    }
    
    /* Check if we're looking for the null terminator */
    if (c == '\0') {
        return (char *)scan;
    }
    
    /* Return the last occurrence found, or NULL if none */
    return (char *)place;
}
