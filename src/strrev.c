/*
 * SPDX-License-Identifier: BSD-2-Clause
 * strrev.c - Reverse string in place
 *
 * Copyright (C) 2025 by amigazen project
 *
 * This function reverses the characters in a string in place.
 * The string must be null-terminated.
 */

#include <string.h>
#include "include/string.h"

/*
 * strrev() - Reverse string in place
 * 
 * Reverses the characters in a string in place.
 * The string must be null-terminated.
 *
 * Parameters:
 *   str - String to reverse
 *
 * Returns:
 *   Pointer to the original string (for chaining)
 */
char *strrev(char *str)
{
    char *start = str;
    char *end = str + strlen(str) - 1;
    char temp;
    
    /* Reverse characters from both ends */
    while (start < end) {
        temp = *start;
        *start = *end;
        *end = temp;
        start++;
        end--;
    }
    
    return str;
}
