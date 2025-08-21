/*
 * strlcat.c - safe string concatenation (POSIX compliant)
 *
 * This function appends the null-terminated string src to the end of dst.
 * It will write at most size-1 bytes, null-terminating the result.
 *
 * POSIX.1-2001, POSIX.1-2008, 4.3BSD
 */

#include "amiga.h"
#include <string.h>

size_t strlcat(char *dst, const char *src, size_t size)
{
    size_t dst_len, src_len, copy_len;
    
    chkabort();
    
    if (dst == NULL || src == NULL) {
        return 0;
    }
    
    dst_len = strlen(dst);
    src_len = strlen(src);
    
    if (dst_len >= size) {
        return size + src_len;
    }
    
    copy_len = size - dst_len - 1;
    if (src_len < copy_len) {
        copy_len = src_len;
    }
    
    memcpy(dst + dst_len, src, copy_len);
    dst[dst_len + copy_len] = '\0';
    
    return dst_len + src_len;
}
