/*
 * SPDX-License-Identifier: BSD-2-Clause
 * strichr.c - Case-insensitive character search
 *
 * Copyright (C) 2025 by amigazen project
 *
 * This function searches for a character in a string, ignoring case.
 * Returns a pointer to the first occurrence of the character.
 */

#include <ctype.h>
#include "include/string.h"

/*
 * strichr() - Case-insensitive character search
 * 
 * Searches for a character in a string, ignoring case.
 * Returns a pointer to the first occurrence of the character.
 *
 * Parameters:
 *   str - String to search in
 *   c   - Character to search for
 *
 * Returns:
 *   Pointer to first occurrence, or NULL if not found
 */
char *strichr(char *str, int c)
{
    int oc;
    
    /* Get the opposite case of the search character */
    if (isupper(c)) {
        oc = tolower(c);
    } else {
        oc = toupper(c);
    }
    
    /* Search for either case */
    while (*str) {
        if (*str == c || *str == oc) {
            return str;
        }
        str++;
    }
    
    return NULL;
}
