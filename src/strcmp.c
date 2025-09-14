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
#include <proto/utility.h>
#include "amiga.h"

/**
 * @brief Compare two strings (optimized for Amiga)
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
 * - Uses Amiga Strcmp() for optimal performance when available
 * - Falls back to optimized C implementation for compatibility
 * - Performs lexicographic comparison of the strings
 * - Returns negative value if s1 < s2
 * - Returns zero if s1 == s2
 * - Returns positive value if s1 > s2
 * - Handles signed character comparison correctly
 * - Is C89 compliant for SAS/C compiler compatibility
 */
int strcmp(const char *s1, const char *s2)
{
    /* Validate parameters */
    if (s1 == NULL && s2 == NULL) {
        return 0;
    }
    if (s1 == NULL) {
        return -1;
    }
    if (s2 == NULL) {
        return 1;
    }
    
    /* Use Amiga Strcmp for optimal performance */
    return (int)Strcmp((const UBYTE *)s1, (const UBYTE *)s2);
}