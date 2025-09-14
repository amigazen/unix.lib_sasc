/*
 * SPDX-License-Identifier: BSD-2-Clause
 * tolower.c - Convert character to lowercase (POSIX compliant)
 *
 * This function converts an uppercase letter to the corresponding lowercase
 * letter. If the argument is not an uppercase letter, it is returned unchanged.
 *
 * POSIX.1-2001, POSIX.1-2008, C89, C99
 * Copyright (C) 2025 by amigazen project
 */

#include "amiga.h"
#include <proto/utility.h>

/**
 * @brief Convert character to lowercase
 * @param c Character to convert
 * @return Lowercase equivalent of c, or c unchanged if not an uppercase letter
 * 
 * The tolower() function converts an uppercase letter to the corresponding
 * lowercase letter. If the argument is not an uppercase letter, it is
 * returned unchanged.
 * 
 * This implementation:
 * - Uses utility.library ToLower() for international character set support
 * - Falls back to built-in ASCII conversion if utility.library unavailable
 * - Is C89 compliant for SAS/C compiler compatibility
 * - Handles locale-specific conversions when locale.library is present
 */
int tolower(int c)
{
    /* Use utility.library ToLower() for international character support */
    if (UtilityBase != NULL) {
        return (int)ToLower((UBYTE)c);
    }
    
    /* Fallback to built-in ASCII conversion if utility.library unavailable */
    if (c >= 'A' && c <= 'Z') {
        return c - 'A' + 'a';
    }
    
    /* Return character unchanged if not uppercase letter */
    return c;
}
