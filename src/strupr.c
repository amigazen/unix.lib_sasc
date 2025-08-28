/*
 * SPDX-License-Identifier: BSD-2-Clause
 * strupr.c - Convert string to uppercase in place
 *
 * Copyright (C) 2025 by amigazen project
 *
 * This function converts all characters in a string to uppercase.
 * The conversion is done in-place, modifying the original string.
 */

#include <ctype.h>
#include "include/string.h"

/*
 * strupr() - Convert string to uppercase in place
 * 
 * Converts all characters in the string to uppercase.
 * The conversion is done in-place, modifying the original string.
 *
 * Parameters:
 *   str - Pointer to the string to convert
 *
 * Returns:
 *   Pointer to the original string (for chaining)
 */
char *strupr(char *str)
{
    char *strbase = str;
    while (*str) {
        *str = toupper(*str);
        str++;
    }
    return strbase;
}
