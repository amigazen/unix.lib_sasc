/*
 * strnlen.c - get string length with limit (POSIX compliant)
 *
 * This function returns the number of characters in the string pointed
 * to by s, excluding the terminating null byte ('\0'), but at most
 * maxlen. In doing this, strnlen() looks only at the first maxlen
 * characters in s, never beyond s+maxlen.
 *
 * POSIX.1-2008, 4.3BSD
 */

#include <string.h>

size_t strnlen(const char *s, size_t maxlen)
{
    const char *p;
    
    if (s == NULL) {
        return 0;
    }
    
    for (p = s; maxlen > 0 && *p != '\0'; p++, maxlen--) {
        /* Continue until we hit the limit or null terminator */
    }
    
    return p - s;
}
