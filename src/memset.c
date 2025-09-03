/*
 * SPDX-License-Identifier: BSD-2-Clause
 * memset.c - fill memory with a constant byte (POSIX compliant)
 *
 * This function fills the first 'n' bytes of the memory area pointed to by 's'
 * with the constant byte 'c'.
 *
 * POSIX.1-2001, POSIX.1-2008, C89, C99
 * Copyright (C) 2025 by amigazen project
 */

#include <string.h>

/**
 * @brief Fill memory with a constant byte
 * @param s Pointer to the memory area to fill
 * @param c Value to be set (converted to unsigned char)
 * @param n Number of bytes to be set
 * @return Pointer to the memory area s
 * 
 * The memset() function fills the first 'n' bytes of the memory area pointed to
 * by 's' with the constant byte 'c'.
 * 
 * This implementation:
 * - Fills exactly n bytes with the value c
 * - Returns a pointer to the memory area s
 * - Treats c as unsigned char for setting
 * - Is optimized for performance with register variables
 * - Is C89 compliant for SAS/C compiler compatibility
 */
void *memset(void *s, int c, size_t n)
{
    char *scan;
    unsigned char uc;
    size_t i;
    
    scan = (char *)s;
    uc = (unsigned char)c;
    
    /* Fill the memory area */
    for (i = 0; i < n; i++) {
        scan[i] = uc;
    }
    
    return s;
}
