/*
 * SPDX-License-Identifier: BSD-2-Clause
 * strspn.c - find length of initial segment of s consisting entirely of characters from accept (POSIX compliant)
 *
 * This function calculates the length of the initial segment of 's' which consists
 * entirely of characters in 'accept'.
 *
 * POSIX.1-2001, POSIX.1-2008
 * Copyright (C) 2025 by amigazen project
 */

#include <string.h>

/**
 * @brief Find length of initial segment consisting of characters from accept
 * @param s String to search in
 * @param accept String containing characters to accept
 * @return Length of initial segment of s consisting entirely of characters from accept
 * 
 * The strspn() function calculates the length of the initial segment of 's' which
 * consists entirely of characters in 'accept'.
 * 
 * This implementation:
 * - Returns the number of characters at the beginning of s that are in accept
 * - Returns 0 if the first character of s is not in accept
 * - Handles empty accept string (returns 0)
 * - Is optimized with early termination
 */
size_t strspn(const char *s, const char *accept)
{
    const char *sscan;
    const char *ascan;
    size_t count;
    
    count = 0;
    
    /* Search through each character in s */
    for (sscan = s; *sscan != '\0'; sscan++) {
        /* Check if current character is in accept */
        for (ascan = accept; *ascan != '\0'; ascan++) {
            if (*sscan == *ascan) {
                /* Character found in accept, increment count and continue */
                count++;
                break;
            }
        }
        
        /* If character not found in accept, we've reached the end of the span */
        if (*ascan == '\0') {
            return count;
        }
    }
    
    /* All characters in s were in accept */
    return count;
}
