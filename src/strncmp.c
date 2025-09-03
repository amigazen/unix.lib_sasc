/*
 * SPDX-License-Identifier: BSD-2-Clause
 * strncmp.c - compare strings with length limit using AmigaOS native function
 *
 * This implementation uses the AmigaOS locale.library StrnCmp() function
 * which provides localized string comparison with proper collation support.
 *
 * POSIX.1-2001, POSIX.1-2008, C89, C99
 * Copyright (C) 2025 by amigazen project
 */

#include <proto/locale.h>
#include <proto/dos.h>
#include <proto/utility.h>
#include <string.h>

/**
 * @brief Compare strings with length limit using AmigaOS native function
 * @param s1 Pointer to the first string
 * @param s2 Pointer to the second string
 * @param n Maximum number of characters to compare
 * @return Integer less than, equal to, or greater than zero if s1 is found,
 *         respectively, to be less than, to match, or be greater than s2
 * 
 * The strncmp() function compares the first 'n' characters of the strings 's1' and 's2'.
 * It returns an integer less than, equal to, or greater than zero if the first 'n' bytes
 * of 's1' is found, respectively, to be less than, to match, or be greater than the
 * first 'n' bytes of 's2'.
 * 
 * This implementation:
 * - Uses AmigaOS locale.library StrnCmp() for optimal performance and localization
 * - Falls back to manual comparison if locale.library is not available
 * - Handles edge cases properly (zero length, null pointers)
 * - Returns negative value if s1 < s2
 * - Returns zero if s1 == s2
 * - Returns positive value if s1 > s2
 * - Is C89 compliant for SAS/C compiler compatibility
 * - Provides ASCII-based case-sensitive comparison (SC_ASCII type)
 */
int strncmp(const char *s1, const char *s2, size_t n)
{
    const char *scan1;
    const char *scan2;
    size_t count;
    
    /* Handle edge cases */
    if (s1 == NULL || s2 == NULL) {
        if (s1 == s2) return 0;
        if (s1 == NULL) return -1;
        return 1;
    }
    
    if (n == 0) {
        return 0;
    }
    
    /* Try to use AmigaOS native StrnCmp for better performance and localization */
    if (LocaleBase != NULL) {
        LONG result;
        struct Locale *locale;
        
        /* Get the default locale */
        locale = OpenLocale(NULL);
        if (locale != NULL) {
            /* Use AmigaOS StrnCmp with ASCII comparison for POSIX compatibility */
            result = StrnCmp(locale, (STRPTR)s1, (STRPTR)s2, (LONG)n, SC_ASCII);
            CloseLocale(locale);
            return (int)result;
        }
    }
    
    /* Fallback to manual implementation if locale.library is not available */
    scan1 = s1;
    scan2 = s2;
    count = n;
    
    /* Compare characters until we find a difference or reach n */
    while (--count >= 0 && *scan1 != '\0' && *scan1 == *scan2) {
        scan1++;
        scan2++;
    }
    
    if (count < 0) {
        return 0;
    }
    
    /*
     * The following case analysis is necessary so that characters
     * which look negative collate low against normal characters but
     * high against the end-of-string NUL.
     */
    if (*scan1 == '\0' && *scan2 == '\0') {
        return 0;
    } else if (*scan1 == '\0') {
        return -1;
    } else if (*scan2 == '\0') {
        return 1;
    } else {
        return (*scan1 - *scan2);
    }
}
