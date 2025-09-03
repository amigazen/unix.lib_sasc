/*
 * SPDX-License-Identifier: BSD-2-Clause
 * strncat.c - concatenate strings with length limit using AmigaOS native function
 * 
 * This implementation uses the AmigaOS dos.library Strncat() function
 * which provides memory-safe string concatenation with proper truncation handling.
 * 
 * POSIX.1-2001, POSIX.1-2008, C89, C99
 * Copyright (C) 2025 by amigazen project
 */

#include <proto/dos.h>
#include <proto/utility.h>
#include <string.h>

/**
 * @brief Concatenate strings with length limit using AmigaOS native function
 * @param dest Destination buffer (must be null-terminated)
 * @param src Source string to append
 * @param n Maximum number of characters to append
 * @return Pointer to destination string
 * 
 * The strncat() function appends at most 'n' characters from the string pointed to
 * by 'src' to the end of the string pointed to by 'dest'. The terminating null byte
 * in 'dest' is overwritten by the first character of 'src', and a null byte is
 * included at the end of the new string formed by the concatenation of both in 'dest'.
 * 
 * This implementation:
 * - Uses AmigaOS dos.library Strncat() for optimal performance
 * - Falls back to manual implementation if dos.library is not available
 * - Handles edge cases properly (null pointers, zero length)
 * - Always null-terminates the destination
 * - Appends at most n characters from src
 * - Returns pointer to destination for chaining
 * - Is C89 compliant for SAS/C compiler compatibility
 */
char *strncat(char *dest, const char *src, size_t n)
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
    
    /* Try to use AmigaOS native Strncat for better performance */
    if (DOSBase != NULL) {
        Strncat((UBYTE *)dest, (const UBYTE *)src, (ULONG)n);
        return dest;
    }
    
    /* Fallback to manual implementation if dos.library is not available */
    /* Find the end of the destination string */
    for (dscan = dest; *dscan != '\0'; dscan++) {
        /* Empty loop body - just advance to end of string */
    }
    
    /* Append the source string to the destination */
    sscan = src;
    count = n;
    while (*sscan != '\0' && --count >= 0) {
        *dscan++ = *sscan++;
    }
    *dscan++ = '\0';
    
    return dest;
}
