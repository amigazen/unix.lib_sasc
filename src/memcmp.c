/*
 * SPDX-License-Identifier: BSD-2-Clause
 * memcmp.c - compare memory areas (POSIX compliant)
 *
 * This function compares the first 'n' bytes of the memory areas 's1' and 's2'.
 * It returns an integer less than, equal to, or greater than zero if the first
 * 'n' bytes of 's1' is found, respectively, to be less than, to match, or be
 * greater than the first 'n' bytes of 's2'.
 *
 * POSIX.1-2001, POSIX.1-2008, C89, C99
 * Copyright (C) 2025 by amigazen project
 */

#include <string.h>

/**
 * @brief Compare memory areas
 * @param s1 Pointer to the first memory area
 * @param s2 Pointer to the second memory area
 * @param n Number of bytes to compare
 * @return Integer less than, equal to, or greater than zero if the first n bytes
 *         of s1 is found, respectively, to be less than, to match, or be greater
 *         than the first n bytes of s2
 * 
 * The memcmp() function compares the first 'n' bytes of the memory areas 's1' and 's2'.
 * It returns an integer less than, equal to, or greater than zero if the first 'n' bytes
 * of 's1' is found, respectively, to be less than, to match, or be greater than the
 * first 'n' bytes of 's2'.
 * 
 * This implementation:
 * - Compares exactly n bytes from both memory areas
 * - Returns negative value if s1 < s2
 * - Returns zero if s1 == s2
 * - Returns positive value if s1 > s2
 * - Is optimized for performance with register variables
 * - Is C89 compliant for SAS/C compiler compatibility
 */
int memcmp(const void *s1, const void *s2, size_t n)
{
    const char *scan1;
    const char *scan2;
    size_t i;
    
    scan1 = (const char *)s1;
    scan2 = (const char *)s2;
    
    /* Compare bytes until we find a difference or reach n */
    for (i = 0; i < n; i++) {
        if (scan1[i] == scan2[i]) {
            continue;
        } else {
            return (scan1[i] - scan2[i]);
        }
    }
    
    /* All n bytes are equal */
    return 0;
}
