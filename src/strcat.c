/*
 * SPDX-License-Identifier: BSD-2-Clause
 * strcat.c - concatenate two strings (POSIX compliant)
 *
 * This function appends the string pointed to by 'src' to the end of the string
 * pointed to by 'dest'. The terminating null byte in 'dest' is overwritten by
 * the first character of 'src', and a null byte is included at the end of the
 * new string formed by the concatenation of both in 'dest'.
 *
 * POSIX.1-2001, POSIX.1-2008, C89, C99
 * Copyright (C) 2025 by amigazen project
 */

#include <string.h>
#include <proto/utility.h>
#include "amiga.h"


/**
 * @brief Concatenate two strings (optimized for Amiga)
 * @param dest Pointer to the destination string (must be null-terminated)
 * @param src Pointer to the source string to append
 * @return Pointer to the destination string (dest)
 * 
 * The strcat() function appends the string pointed to by 'src' to the end of
 * the string pointed to by 'dest'. The terminating null byte in 'dest' is
 * overwritten by the first character of 'src', and a null byte is included
 * at the end of the new string formed by the concatenation of both in 'dest'.
 * 
 * This implementation:
 * - Uses Amiga Strncat() for optimal performance when available
 * - Falls back to optimized C implementation for compatibility
 * - Appends the source string to the end of the destination string
 * - Returns a pointer to the destination string
 * - Does not perform bounds checking (caller must ensure sufficient space)
 * - Is C89 compliant for SAS/C compiler compatibility
 * 
 * Warning: The destination buffer must be large enough to hold both strings
 * plus the terminating null character. No bounds checking is performed.
 */
char *strcat(char *dest, const char *src)
{
    /* Validate parameters */
    if (dest == NULL || src == NULL) {
        return dest;
    }
    
    /* Use Amiga Strncat for optimal performance */
    /* We use a large buffer size since strcat() doesn't have size limits */
    if (Strncat((UBYTE *)dest, (const UBYTE *)src, 0x7FFFFFFF) == NULL) {
        /* Fallback to C implementation if Strncat fails */
        char *dscan;
        const char *sscan;
        
        /* Find the end of the destination string */
        for (dscan = dest; *dscan != '\0'; dscan++) {
            /* Empty loop body - just advance to end of string */
        }
        
        /* Append the source string to the destination */
        sscan = src;
        while ((*dscan++ = *sscan++) != '\0') {
            /* Empty loop body - assignment and increment happen in condition */
        }
    }
    
    return dest;
}

/**
 * @brief Concatenate two strings with length limit (safer alternative)
 * @param dest Pointer to the destination string (must be null-terminated)
 * @param src Pointer to the source string to append
 * @param size Size of the destination buffer
 * @return Length of the string that would have been created
 * 
 * This is a safer version of strcat that limits the number of characters appended
 * to prevent buffer overflows. It's similar to strncat but guarantees null termination.
 * 
 * This implementation:
 * - Uses Amiga Strncat() for optimal performance
 * - Prevents buffer overflows by limiting append length
 * - Always null-terminates the destination string
 * - Returns the length that would have been created (like BSD strlcat)
 * - Is C89 compliant for SAS/C compiler compatibility
 */
size_t strlcat(char *dest, const char *src, size_t size)
{
    UBYTE *result;
    size_t dest_len, src_len, total_len;
    
    /* Validate parameters */
    if (dest == NULL || src == NULL || size == 0) {
        return 0;
    }
    
    /* Calculate source length */
    src_len = strlen(src);
    
    /* Calculate destination length (up to size-1 to leave room for null terminator) */
    dest_len = strnlen(dest, size - 1);
    
    /* Calculate total length that would be created */
    total_len = dest_len + src_len;
    
    /* Use Amiga Strncat for optimal performance */
    result = Strncat((UBYTE *)dest, (const UBYTE *)src, (ULONG)size);
    
    /* Return total length (like BSD strlcat) */
    return total_len;
}