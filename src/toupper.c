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
 * - Converts 'a'-'z' to 'A'-'Z'
 * - Returns other characters unchanged
 * - Is C89 compliant for SAS/C compiler compatibility
 * - Uses simple arithmetic for efficiency
 */
int toupper(int c)
{
    /* Convert lowercase letter to uppercase */
    if (c >= 'a' && c <= 'z') {
        return c - 'a' + 'A';
    }
    
    /* Return character unchanged if not lowercase letter */
    return c;
}
