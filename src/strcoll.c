/*
 * strcoll.c - locale-specific string comparison (POSIX compliant)
 *
 * This function compares the two strings s1 and s2. It returns an
 * integer less than, equal to, or greater than zero if s1 is found,
 * respectively, to be less than, to match, or be greater than s2.
 * On AmigaOS, this is equivalent to strcmp().
 *
 * POSIX.1-2001, POSIX.1-2008, SVr4, 4.3BSD
 */

#include "amiga.h"
#include <string.h>

int strcoll(const char *s1, const char *s2)
{
    chkabort();
    
    if (s1 == NULL || s2 == NULL) {
        return 0;
    }
    
    /* On AmigaOS, we don't have locale support, so use strcmp */
    return strcmp(s1, s2);
}
