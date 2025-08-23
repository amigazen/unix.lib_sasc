/*
 * strncasecmp.c - case-insensitive string comparison with length limit (POSIX compliant)
 *
 * This function performs a case-insensitive comparison of the strings
 * s1 and s2, comparing at most n characters.
 *
 * POSIX.1-2001, POSIX.1-2008, 4.4BSD
 */

#include <strings.h>
#include <ctype.h>

int strncasecmp(const char *s1, const char *s2, size_t n)
{
    int c1, c2;
    
    if (s1 == NULL || s2 == NULL || n == 0) {
        return 0;
    }
    
    while (n > 0 && *s1 && *s2) {
        c1 = tolower((unsigned char)*s1);
        c2 = tolower((unsigned char)*s2);
        
        if (c1 != c2) {
            return c1 - c2;
        }
        
        s1++;
        s2++;
        n--;
    }
    
    if (n == 0) {
        return 0;
    }
    
    return tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
}
