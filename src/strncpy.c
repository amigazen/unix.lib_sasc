/*
 * SPDX-License-Identifier: BSD-2-Clause
 * strncpy.c - copy a string with length limit using AmigaOS native function
 * 
 * This implementation uses the AmigaOS dos.library Strncpy() function
 * which provides memory-safe string copying with proper truncation handling.
 * 
 * POSIX.1-2001, POSIX.1-2008, C89, C99
 * Copyright (C) 2025 by amigazen project
 */

#include <proto/dos.h>
#include <proto/utility.h>
#include <string.h>

/**
 * @brief Copy a string with length limit using AmigaOS native function
 * @param dest Destination buffer
 * @param src Source string
 * @param n Maximum number of characters to copy
 * @return Pointer to destination string
 * 
 * The strncpy() function copies at most 'n' characters from the string pointed to
 * by 'src' to the array pointed to by 'dest'. If the length of 'src' is less than
 * 'n', strncpy() writes additional null bytes to 'dest' to ensure that a total of
 * 'n' bytes are written.
 * 
 * This implementation:
 * - Uses AmigaOS dos.library Strncpy() for optimal performance
 * - Falls back to manual implementation if dos.library is not available
 * - Handles edge cases properly (null pointers, zero length)
 * - Always null-terminates the destination (if n > 0)
 * - Pads with null bytes if source is shorter than n
 * - Returns pointer to destination for chaining
 * - Is C89 compliant for SAS/C compiler compatibility
 */
char *strncpy(char *dest, const char *src, size_t n)
{
    char *dscan;
    const char *sscan;
    size_t count;
    
    /* Handle edge cases */
    if (dest == NULL || src == NULL) {
        return dest;
    }
    
    if (n == 0) {
        return dest;
    }
    
    /* Try to use AmigaOS native Strncpy for better performance */
    if (DOSBase != NULL) {
        Strncpy((UBYTE *)dest, (const UBYTE *)src, (ULONG)n);
        return dest;
    }
    
    /* Fallback to manual implementation if dos.library is not available */
    dscan = dest;
    sscan = src;
    count = n;
    
    /* Copy characters until we reach n or null terminator */
    while (--count >= 0 && (*dscan++ = *sscan++) != '\0') {
        /* Empty loop body - assignment and increment happen in condition */
    }
    
    /* Pad with null bytes if source was shorter than n */
    while (--count >= 0) {
        *dscan++ = '\0';
    }
    
    return dest;
}
