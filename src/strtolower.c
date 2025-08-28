/*
 * SPDX-License-Identifier: BSD-2-Clause
 * strtolower.c - Convert string to lowercase in place
 *
 * Copyright (C) 2025 by amigazen project
 *
 * This function converts all characters in a string to lowercase.
 * The conversion is done in-place, modifying the original string.
 */

#include <ctype.h>
#include "include/string.h"

/*
 * strtolower() - Convert string to lowercase in place
 * 
 * Converts all characters in the string to lowercase.
 * The conversion is done in-place, modifying the original string.
 *
 * Parameters:
 *   str - Pointer to the string to convert
 *
 * Returns:
 *   void (modifies string in place)
 */
void strtolower(char *str)
{
    while (*str) {
        *str = tolower(*str);
        str++;
    }
}
