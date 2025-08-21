/*
 * strdup.c - duplicate a string (POSIX compliant)
 *
 * This function returns a pointer to a new string which is a duplicate
 * of the string s. Memory for the new string is obtained with malloc(),
 * and can be freed with free().
 *
 * POSIX.1-2001, POSIX.1-2008, SVr4, 4.3BSD
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>

char *strdup(const char *s)
{
    char *dup;
    size_t len;
    
    if (s == NULL) {
        errno = EINVAL;
        return NULL;
    }
    
    len = strlen(s) + 1;
    dup = malloc(len);
    
    if (dup == NULL) {
        return NULL;  /* errno already set by malloc */
    }
    
    memcpy(dup, s, len);
    return dup;
}
