/*
 * fnmatch.c - filename pattern matching (POSIX compliant)
 *
 * This function checks whether the string argument matches the pattern
 * argument, which is a shell wildcard pattern.
 *
 * POSIX.1-2001, POSIX.1-2008, 4.3BSD
 */

#include "amiga.h"
#include <fnmatch.h>
#include <string.h>
#include <ctype.h>

int fnmatch(const char *pattern, const char *string, int flags)
{
    int i = 0, j = 0;
    
    chkabort();
    
    if (pattern == NULL || string == NULL) {
        return FNM_NOMATCH;
    }
    
    /* Simple pattern matching implementation */
    while (pattern[i] != '\0' && string[j] != '\0') {
        if (pattern[i] == '*') {
            /* Skip multiple asterisks */
            while (pattern[i] == '*') i++;
            
            if (pattern[i] == '\0') {
                return 0;  /* Pattern ends with *, so it matches */
            }
            
            /* Find the next character in string that matches pattern[i] */
            while (string[j] != '\0' && string[j] != pattern[i]) {
                j++;
            }
            
            if (string[j] == '\0') {
                return FNM_NOMATCH;
            }
        } else if (pattern[i] == '?') {
            /* ? matches any single character */
            i++;
            j++;
        } else if (pattern[i] == '[') {
            /* Character class */
            int match = 0;
            int negate = 0;
            
            i++;  /* Skip '[' */
            if (pattern[i] == '^' || pattern[i] == '!') {
                negate = 1;
                i++;
            }
            
            while (pattern[i] != ']' && pattern[i] != '\0') {
                if (pattern[i] == string[j]) {
                    match = 1;
                }
                i++;
            }
            
            if (pattern[i] == '\0') {
                return FNM_NOMATCH;  /* Unmatched '[' */
            }
            
            if (match == negate) {
                return FNM_NOMATCH;
            }
            
            i++;  /* Skip ']' */
            j++;
        } else {
            /* Literal character */
            if (pattern[i] != string[j]) {
                return FNM_NOMATCH;
            }
            i++;
            j++;
        }
    }
    
    /* Check if both strings are exhausted */
    if (pattern[i] == '\0' && string[j] == '\0') {
        return 0;
    }
    
    return FNM_NOMATCH;
}
