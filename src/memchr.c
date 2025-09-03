/*
 * SPDX-License-Identifier: BSD-2-Clause
 * memchr.c - scan memory for a character (POSIX compliant)
 *
 * This function scans the first 'n' bytes of the memory area pointed to by 's'
 * for the first instance of 'c'. Both 'c' and the bytes of the memory area
 * pointed to by 's' are interpreted as unsigned char.
 *
 * POSIX.1-2001, POSIX.1-2008, C89, C99
 * Copyright (C) 2025 by amigazen project
 */

#include <string.h>

/**
 * @brief Scan memory for a character
 * @param s Pointer to the memory area to scan
 * @param c Character to search for (converted to unsigned char)
 * @param n Number of bytes to scan
 * @return Pointer to the matching byte, or NULL if the character does not occur
 *         in the first n bytes of the memory area
 * 
 * The memchr() function scans the first 'n' bytes of the memory area pointed to
 * by 's' for the first instance of 'c'. Both 'c' and the bytes of the memory area
 * pointed to by 's' are interpreted as unsigned char.
 * 
 * This implementation:
 * - Scans exactly n bytes for the character c
 * - Returns pointer to the first occurrence of c
 * - Returns NULL if c is not found within n bytes
 * - Treats c as unsigned char for comparison
 * - Is optimized for performance with register variables
 * - Is C89 compliant for SAS/C compiler compatibility
 */
void *memchr(const void *s, int c, size_t n)
{
    const char *scan;
    unsigned char uc;
    size_t i;
    
    scan = (const char *)s;
    uc = (unsigned char)c;
    
    /* Scan through the memory area */
    for (i = 0; i < n; i++) {
        if ((unsigned char)scan[i] == uc) {
            return (void *)(scan + i);
        }
    }
    
    /* Character not found */
    return NULL;
}
