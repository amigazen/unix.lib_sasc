/*
 * strcasecmp.c - case-insensitive string comparison (POSIX compliant)
 *
 * This function performs a case-insensitive comparison of the strings
 * s1 and s2. It returns an integer less than, equal to, or greater
 * than zero if s1 is found, respectively, to be less than, to match,
 * or be greater than s2.
 *
 * POSIX.1-2001, POSIX.1-2008, 4.4BSD
 */

#include <strings.h>
#include <ctype.h>

int strcasecmp(const char *s1, const char *s2)
{
    int c1, c2;
    
    if (s1 == NULL || s2 == NULL) {
        return 0;
    }
    
    while (*s1 && *s2) {
        c1 = tolower((unsigned char)*s1);
        c2 = tolower((unsigned char)*s2);
        
        if (c1 != c2) {
            return c1 - c2;
        }
        
        s1++;
        s2++;
    }
    
    return tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
}
