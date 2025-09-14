/*
 * SPDX-License-Identifier: BSD-2-Clause
 * toupper.c - Convert character to uppercase (POSIX compliant)
 *
 * This function converts a lowercase letter to the corresponding uppercase
 * letter. If the argument is not a lowercase letter, it is returned unchanged.
 *
 * POSIX.1-2001, POSIX.1-2008, C89, C99
 * Copyright (C) 2025 by amigazen project
 */

#include "amiga.h"
#include <proto/utility.h>

/**
 * @brief Convert character to uppercase
 * @param c Character to convert
 * @return Uppercase equivalent of c, or c unchanged if not a lowercase letter
 * 
 * The toupper() function converts a lowercase letter to the corresponding
 * uppercase letter. If the argument is not a lowercase letter, it is
 * returned unchanged.
 * 
 * This implementation:
 * - Uses utility.library ToUpper() for international character set support
 * - Falls back to built-in ASCII conversion if utility.library unavailable
 * - Is C89 compliant for SAS/C compiler compatibility
 * - Handles locale-specific conversions when locale.library is present
 */
int toupper(int c)
{
    /* Use utility.library ToUpper() for international character support */
    if (UtilityBase != NULL) {
        return (int)ToUpper((UBYTE)c);
    }
    
    /* Fallback to built-in ASCII conversion if utility.library unavailable */
    if (c >= 'a' && c <= 'z') {
        return c - 'a' + 'A';
    }
    
    /* Return character unchanged if not lowercase letter */
    return c;
}
