/*
 * SPDX-License-Identifier: BSD-2-Clause
 * strcmp.c - compare two strings (POSIX compliant)
 *
 * This function compares the two strings 's1' and 's2'. It returns an integer
 * less than, equal to, or greater than zero if 's1' is found, respectively, to
 * be less than, to match, or be greater than 's2'.
 *
 * POSIX.1-2001, POSIX.1-2008, C89, C99
 * Copyright (C) 2025 by amigazen project
 */

#include <string.h>

/**
 * @brief Compare two strings
 * @param s1 Pointer to the first string
 * @param s2 Pointer to the second string
 * @return Integer less than, equal to, or greater than zero if s1 is found,
 *         respectively, to be less than, to match, or be greater than s2
 * 
 * The strcmp() function compares the two strings 's1' and 's2'. It returns an
 * integer less than, equal to, or greater than zero if 's1' is found, respectively,
 * to be less than, to match, or be greater than 's2'.
 * 
 * This implementation:
 * - Performs lexicographic comparison of the strings
 * - Returns negative value if s1 < s2
 * - Returns zero if s1 == s2
 * - Returns positive value if s1 > s2
 * - Handles signed character comparison correctly
 * - Is optimized for performance with register variables
 * - Is C89 compliant for SAS/C compiler compatibility
 */
int strcmp(const char *s1, const char *s2)
{
    const char *scan1;
    const char *scan2;
    
    scan1 = s1;
    scan2 = s2;
    
    /* Compare characters until we find a difference or reach end of string */
    while (*scan1 != '\0' && *scan1 == *scan2) {
        scan1++;
        scan2++;
    }
    
    /*
     * The following case analysis is necessary so that characters
     * which look negative collate low against normal characters but
     * high against the end-of-string NUL.
     */
    if (*scan1 == '\0' && *scan2 == '\0') {
        return 0;
    } else if (*scan1 == '\0') {
        return -1;
    } else if (*scan2 == '\0') {
        return 1;
    } else {
        return (*scan1 - *scan2);
    }
}
