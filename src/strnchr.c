/*
 * SPDX-License-Identifier: BSD-2-Clause
 * strnchr.c - Bounded character search
 *
 * Copyright (C) 2025 by amigazen project
 *
 * This function searches for a character in a string within a specified length.
 * Useful for searching in strings that may not be null-terminated
 * or when you want to limit the search range.
 */

#include "include/string.h"

/*
 * strnchr() - Bounded character search
 * 
 * Searches for a character in a string within a specified length.
 * Useful for searching in strings that may not be null-terminated
 * or when you want to limit the search range.
 *
 * Parameters:
 *   str - String to search in
 *   c   - Character to search for
 *   len - Maximum length to search
 *
 * Returns:
 *   Pointer to first occurrence, or NULL if not found
 */
char *strnchr(char *str, int c, int len)
{
    char *donestr = (char *)((int)str + len);
    
    while (str < donestr) {
        if (*str == c) {
            return str;
        }
        str++;
    }
    
    return NULL;
}
