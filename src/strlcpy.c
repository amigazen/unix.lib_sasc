/*
 * strlcpy.c - safe string copy (POSIX compliant)
 *
 * This function copies the string src to dst (including the terminating
 * null byte) and returns the length of src. It is designed to be safer,
 * more consistent, and less error prone replacements for strncpy().
 *
 * POSIX.1-2001, POSIX.1-2008, 4.3BSD
 */

#include "amiga.h"
#include <string.h>

size_t strlcpy(char *dst, const char *src, size_t size)
{
    size_t src_len;
    
    chkabort();
    
    if (dst == NULL || src == NULL) {
        return 0;
    }
    
    src_len = strlen(src);
    
    if (size > 0) {
        if (src_len >= size) {
            /* Truncate */
            memcpy(dst, src, size - 1);
            dst[size - 1] = '\0';
        } else {
            /* Copy entire string */
            memcpy(dst, src, src_len + 1);
        }
    }
    
    return src_len;
}
