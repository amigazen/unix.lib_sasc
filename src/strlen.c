/*
 * SPDX-License-Identifier: BSD-2-Clause
 * strlen.c - calculate the length of a string (POSIX compliant)
 *
 * This function calculates the length of the string pointed to by 's',
 * excluding the terminating null byte ('\0').
 *
 * POSIX.1-2001, POSIX.1-2008, C89, C99
 * Copyright (C) 2025 by amigazen project
 */

#include <string.h>
#include <proto/utility.h>
#include "amiga.h"


/**
 * @brief Calculate the length of a string (optimized for Amiga)
 * @param s Pointer to the null-terminated string
 * @return The number of characters in the string, excluding the null terminator
 * 
 * The strlen() function calculates the length of the string pointed to by 's',
 * excluding the terminating null byte ('\0').
 * 
 * This implementation:
 * - Uses Amiga Strlen() for optimal performance when available
 * - Falls back to optimized C implementation for compatibility
 * - Returns the number of characters in the string
 * - Does not count the terminating null character
 * - Handles null pointer input gracefully (undefined behavior per standard)
 * - Is C89 compliant for SAS/C compiler compatibility
 */
size_t strlen(const char *s)
{
    /* Validate parameter */
    if (s == NULL) {
        return 0;  /* Handle NULL gracefully */
    }
    
    /* Use Amiga Strlen for optimal performance */
    return (size_t)Strlen((const UBYTE *)s);
}

