/*
 * strlcpy.c - size-bounded string copy (BSD extension)
 * 
 * This function is designed to be safer, more consistent, and less error prone
 * than strncpy. It guarantees that the destination string is always null-terminated.
 * 
 * BSD extension: guaranteed null-terminated string copy
 */

#include <string.h>

/**
 * @brief Size-bounded string copy with guaranteed null termination
 * @param dst Destination buffer
 * @param src Source string
 * @param size Size of destination buffer
 * @return Length of source string (strlen(src))
 * 
 * This function copies up to size-1 characters from src to dst, always
 * null-terminating the result. The return value is the length of src,
 * which allows the caller to check for truncation.
 * 
 * Unlike strncpy, this function:
 * - Always null-terminates the destination
 * - Returns the length of the source string
 * - Provides a way to detect truncation
 * - Is safer for buffer management
 */
size_t strlcpy(char *dst, const char *src, size_t size)
{
    size_t src_len;
    
    if (dst == NULL || src == NULL) {
        return 0;
    }
    
    src_len = strlen(src);
    
    if (size > 0) {
        if (src_len >= size) {
            /* Truncate and null-terminate */
            memcpy(dst, src, size - 1);
            dst[size - 1] = '\0';
        } else {
            /* Copy entire string and null-terminate */
            memcpy(dst, src, src_len + 1);
        }
    }
    
    return src_len;
}
