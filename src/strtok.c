/*
 * SPDX-License-Identifier: BSD-2-Clause
 * strtok.c - extract tokens from strings (POSIX compliant)
 *
 * This function breaks a string into a sequence of zero or more nonempty tokens.
 * On the first call to strtok(), the string to be parsed should be specified in 's'.
 * In each subsequent call that should parse the same string, 's' should be NULL.
 *
 * POSIX.1-2001, POSIX.1-2008
 * Copyright (C) 2025 by amigazen project
 */

#include <string.h>

/* Static variable to maintain state between calls */
static char *scanpoint = NULL;

/**
 * @brief Extract tokens from strings
 * @param s String to be parsed (NULL for subsequent calls on same string)
 * @param delim String containing delimiter characters
 * @return Pointer to next token, or NULL if no more tokens
 * 
 * The strtok() function breaks a string into a sequence of zero or more nonempty
 * tokens. On the first call to strtok(), the string to be parsed should be specified
 * in 's'. In each subsequent call that should parse the same string, 's' should be NULL.
 * 
 * This implementation:
 * - Uses static storage to maintain state between calls
 * - Modifies the input string by replacing delimiters with null characters
 * - Skips leading delimiters
 * - Returns NULL when no more tokens are available
 * - Is thread-unsafe (use strtok_r for thread safety)
 */
char *strtok(char *s, const char *delim)
{
    char *scan;
    char *tok;
    const char *dscan;
    
    /* If s is NULL and we have no saved scanpoint, return NULL */
    if (s == NULL && scanpoint == NULL) {
        return NULL;
    }
    
    /* Use provided string or continue from saved scanpoint */
    if (s != NULL) {
        scan = s;
    } else {
        scan = scanpoint;
    }
    
    /* Skip leading delimiters */
    for (; *scan != '\0'; scan++) {
        for (dscan = delim; *dscan != '\0'; dscan++) {
            if (*scan == *dscan) {
                break;
            }
        }
        /* If we didn't find a delimiter, we found the start of a token */
        if (*dscan == '\0') {
            break;
        }
    }
    
    /* If we reached end of string, no more tokens */
    if (*scan == '\0') {
        scanpoint = NULL;
        return NULL;
    }
    
    /* Remember start of token */
    tok = scan;
    
    /* Find end of token (next delimiter or end of string) */
    for (; *scan != '\0'; scan++) {
        for (dscan = delim; *dscan != '\0'; dscan++) {
            if (*scan == *dscan) {
                /* Found delimiter, terminate token and save position */
                scanpoint = scan + 1;
                *scan = '\0';
                return tok;
            }
        }
    }
    
    /* Reached end of string, no more tokens */
    scanpoint = NULL;
    return tok;
}
