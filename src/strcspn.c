/*
 * SPDX-License-Identifier: BSD-2-Clause
 * strcspn.c - find length of initial segment of s consisting entirely of characters not from reject (POSIX compliant)
 *
 * This function calculates the length of the initial segment of 's' which consists
 * entirely of characters not in 'reject'.
 *
 * POSIX.1-2001, POSIX.1-2008
 * Copyright (C) 2025 by amigazen project
 */

#include <string.h>

/**
 * @brief Find length of initial segment consisting of characters not from reject
 * @param s String to search in
 * @param reject String containing characters to reject
 * @return Length of initial segment of s consisting entirely of characters not in reject
 * 
 * The strcspn() function calculates the length of the initial segment of 's' which
 * consists entirely of characters not in 'reject'.
 * 
 * This implementation:
 * - Returns the number of characters at the beginning of s that are not in reject
 * - Returns 0 if the first character of s is in reject
 * - Handles empty reject string (returns length of s)
 * - Is optimized with early termination
 */
size_t strcspn(const char *s, const char *reject)
{
    const char *scan;
    const char *rscan;
    size_t count;
    
    count = 0;
    
    /* Search through each character in s */
    for (scan = s; *scan != '\0'; scan++) {
        /* Check if current character is in reject */
        for (rscan = reject; *rscan != '\0'; rscan++) {
            if (*scan == *rscan) {
                /* Character found in reject, we've reached the end of the span */
                return count;
            }
        }
        /* Character not in reject, increment count */
        count++;
    }
    
    /* All characters in s were not in reject */
    return count;
}
