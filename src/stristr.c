/*
 * SPDX-License-Identifier: BSD-2-Clause
 * stristr.c - Case-insensitive substring search
 *
 * Copyright (C) 2025 by amigazen project
 *
 * This function searches for a substring within a string, ignoring case.
 * Returns a pointer to the first occurrence of the substring.
 */

#include <ctype.h>
#include "include/string.h"

/*
 * stristr() - Case-insensitive substring search
 * 
 * Searches for a substring within a string, ignoring case.
 * Returns a pointer to the first occurrence of the substring.
 *
 * Parameters:
 *   bigstr - String to search in
 *   substr  - Substring to search for
 *
 * Returns:
 *   Pointer to first occurrence, or NULL if not found
 */
char *stristr(char *bigstr, char *substr)
{
    while (*bigstr) {
        if (toupper(*bigstr) == toupper(*substr)) {
            char *str = bigstr + 1;
            char *sub = substr + 1;
            
            /* Check if the rest of the substring matches */
            while (*sub && toupper(*sub) == toupper(*str)) {
                sub++;
                str++;
            }
            
            if (!*sub) {
                return bigstr;  /* Found complete match */
            }
        }
        bigstr++;
    }
    
    return NULL;
}
