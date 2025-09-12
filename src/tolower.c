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
 * - Converts 'A'-'Z' to 'a'-'z'
 * - Returns other characters unchanged
 * - Is C89 compliant for SAS/C compiler compatibility
 * - Uses simple arithmetic for efficiency
 */
int tolower(int c)
{
    /* Convert uppercase letter to lowercase */
    if (c >= 'A' && c <= 'Z') {
        return c - 'A' + 'a';
    }
    
    /* Return character unchanged if not uppercase letter */
    return c;
}
