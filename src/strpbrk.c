/*
 * SPDX-License-Identifier: BSD-2-Clause
 * strpbrk.c - find first occurrence of any char from breakat in s (POSIX compliant)
 *
 * This function locates the first occurrence in the string 's' of any character
 * in the string 'breakat'.
 *
 * POSIX.1-2001, POSIX.1-2008
 * Copyright (C) 2025 by amigazen project
 */

#include <string.h>

/**
 * @brief Find first occurrence of any character from a set
 * @param s String to search in
 * @param breakat String containing characters to search for
 * @return Pointer to first occurrence of any character from breakat, or NULL if none found
 * 
 * The strpbrk() function locates the first occurrence in the string 's' of any
 * character in the string 'breakat'.
 * 
 * This implementation:
 * - Returns pointer to the first matching character if found
 * - Returns NULL if no characters from breakat are found in s
 * - Handles empty breakat string (returns NULL)
 * - Is optimized with early termination
 */
char *strpbrk(const char *s, const char *breakat)
{
    const char *sscan;
    const char *bscan;
    
    /* Search through each character in s */
    for (sscan = s; *sscan != '\0'; sscan++) {
        /* Check if current character matches any in breakat */
        for (bscan = breakat; *bscan != '\0'; bscan++) {
            if (*sscan == *bscan) {
                return (char *)sscan;
            }
        }
    }
    
    /* No characters from breakat found in s */
    return NULL;
}
