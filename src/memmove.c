/*
 * SPDX-License-Identifier: BSD-2-Clause
 * memmove.c - move memory area (POSIX compliant)
 *
 * This function moves 'n' bytes from memory area 'src' to memory area 'dest'.
 * The memory areas may overlap. This is the safe version of memcpy().
 *
 * POSIX.1-2001, POSIX.1-2008, C89, C99
 * Copyright (C) 2025 by amigazen project
 */

#include <string.h>

/**
 * @brief Move memory area
 * @param dest Pointer to the destination memory area
 * @param src Pointer to the source memory area
 * @param n Number of bytes to move
 * @return Pointer to the destination memory area (dest)
 * 
 * The memmove() function moves 'n' bytes from memory area 'src' to memory area 'dest'.
 * The memory areas may overlap. This is the safe version of memcpy().
 * 
 * This implementation:
 * - Moves exactly n bytes from src to dest
 * - Handles overlapping memory areas safely
 * - Returns a pointer to the destination memory area
 * - Is C89 compliant for SAS/C compiler compatibility
 * - Uses appropriate copy direction based on overlap detection
 */
void *memmove(void *dest, const void *src, size_t n)
{
    char *d;
    const char *s;
    
    if (n == 0) {
        return dest;
    }
    
    s = (const char *)src;
    d = (char *)dest;
    
    if (d > s) {
        /* Copy from end to beginning to handle overlap */
        s += n - 1;
        d += n - 1;
        while (n > 0) {
            *d-- = *s--;
            n--;
        }
    } else {
        /* Copy from beginning to end */
        while (n > 0) {
            *d++ = *s++;
            n--;
        }
    }
    
    return dest;
}
