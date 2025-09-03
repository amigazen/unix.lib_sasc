/*
 * SPDX-License-Identifier: BSD-2-Clause
 * memcpy.c - copy memory area (POSIX compliant)
 *
 * This function copies 'n' bytes from memory area 'src' to memory area 'dest'.
 * The memory areas must not overlap. Use memmove() if the memory areas do overlap.
 *
 * POSIX.1-2001, POSIX.1-2008, C89, C99
 * Copyright (C) 2025 by amigazen project
 */

#include <string.h>

/**
 * @brief Copy memory area
 * @param dest Pointer to the destination memory area
 * @param src Pointer to the source memory area
 * @param n Number of bytes to copy
 * @return Pointer to the destination memory area (dest)
 * 
 * The memcpy() function copies 'n' bytes from memory area 'src' to memory area 'dest'.
 * The memory areas must not overlap. Use memmove() if the memory areas do overlap.
 * 
 * This implementation:
 * - Copies exactly n bytes from src to dest
 * - Returns a pointer to the destination memory area
 * - Does not handle overlapping memory areas (use memmove for that)
 * - Is optimized for performance with register variables
 * - Is C89 compliant for SAS/C compiler compatibility
 * 
 * Note: This implementation handles overlap detection and uses memmove behavior
 * when overlap is detected, which is safer but may be slower than a pure memcpy.
 */
void *memcpy(void *dest, const void *src, size_t n)
{
    char *d;
    const char *s;
    size_t i;
    
    if (n == 0) {
        return dest;
    }
    
    s = (const char *)src;
    d = (char *)dest;
    
    /* Check for overlap and handle appropriately */
    if (s <= d && s + (n-1) >= d) {
        /* Overlap detected, must copy right-to-left */
        s += n - 1;
        d += n - 1;
        for (i = n; i > 0; i--) {
            *d-- = *s--;
        }
    } else {
        /* No overlap, copy left-to-right */
        for (i = 0; i < n; i++) {
            d[i] = s[i];
        }
    }
    
    return dest;
}
