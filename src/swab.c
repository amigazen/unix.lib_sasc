/*
 * SPDX-License-Identifier: BSD-2-Clause
 * swab.c - copy and swap adjacent bytes (POSIX compliant)
 *
 * This function copies bytes from source to destination, swapping
 * adjacent bytes in pairs. It's useful for converting between
 * different byte orders.
 *
 * POSIX.1-2008, X/Open System Interfaces
 * Copyright (C) 2025 by amigazen project
 */

#include <unistd.h>
#include <stddef.h>
#include <sys/types.h>

/**
 * @brief Copy and swap adjacent bytes
 * @param src Source buffer (unchanged)
 * @param dst Destination buffer
 * @param nbytes Number of bytes to process (must be even)
 * 
 * The swab() function copies 'nbytes' bytes from 'src' to 'dst',
 * swapping adjacent bytes in pairs. The source buffer is unchanged.
 * The number of bytes 'nbytes' should be even.
 * 
 * This implementation:
 * - Copies bytes from src to dst
 * - Swaps adjacent bytes in pairs during copy
 * - Leaves source buffer unchanged
 * - Processes nbytes/2 word pairs
 * - Is POSIX compliant
 */
void swab(const void *src, void *dst, ssize_t nbytes)
{
    const char *s;
    char *d;
    char c;
    ssize_t n;
    
    if (src == NULL || dst == NULL || nbytes <= 0) {
        return;
    }
    
    s = (const char *)src;
    d = (char *)dst;
    n = nbytes >> 1;  /* convert to word count */
    
    /* Copy and swap bytes in pairs from src to dst */
    while (n--) {
        c = *s++;
        *d++ = *s++;
        *d++ = c;
    }
}
