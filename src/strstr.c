/*
 * SPDX-License-Identifier: BSD-2-Clause
 * strstr.c - find first occurrence of wanted in s (POSIX compliant)
 *
 * This function locates the first occurrence of the string 'wanted' in the
 * string 's'. The terminating null characters are not compared.
 *
 * POSIX.1-2001, POSIX.1-2008
 * Copyright (C) 2025 by amigazen project
 */

#include <string.h>

/**
 * @brief Find first occurrence of substring
 * @param s String to search in
 * @param wanted Substring to search for
 * @return Pointer to first occurrence of wanted in s, or NULL if not found
 * 
 * The strstr() function finds the first occurrence of the substring 'wanted'
 * in the string 's'. The terminating null characters are not compared.
 * 
 * This implementation:
 * - Returns pointer to the beginning of the substring if found
 * - Returns NULL if the substring is not found
 * - Handles empty substring (returns s)
 * - Is optimized for performance with early character matching
 */
char *strstr(const char *s, const char *wanted)
{
    const char *scan;
    size_t len;
    char firstc;
    
    /* Handle empty substring case - return s */
    if (*wanted == '\0') {
        return (char *)s;
    }
    
    /* Get the first character and length of wanted string for optimization */
    firstc = *wanted;
    len = strlen(wanted);
    
    /* Search through s for the first character of wanted */
    for (scan = s; *scan != '\0'; scan++) {
        /* If first character matches, check if the full substring matches */
        if (*scan == firstc && strncmp(scan, wanted, len) == 0) {
            return (char *)scan;
        }
    }
    
    /* Substring not found */
    return NULL;
}
