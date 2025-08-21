/*
 * strsep.c - extract token from string (POSIX compliant)
 *
 * This function locates, in the string referenced by *stringp, the
 * first occurrence of any character in the string delim and replaces
 * it with a null byte ('\0').
 *
 * POSIX.1-2001, POSIX.1-2008, 4.3BSD
 */

#include "amiga.h"
#include <string.h>

char *strsep(char **stringp, const char *delim)
{
    char *s, *token;
    
    chkabort();
    
    if (stringp == NULL || *stringp == NULL || delim == NULL) {
        return NULL;
    }
    
    s = *stringp;
    token = s;
    
    /* Find the first delimiter */
    while (*s && !strchr(delim, *s)) {
        s++;
    }
    
    if (*s) {
        *s = '\0';
        *stringp = s + 1;
    } else {
        *stringp = NULL;
    }
    
    return token;
}
