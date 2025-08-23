/*
 * strlcat.c - size-bounded string concatenation (BSD extension)
 * 
 * This function is designed to be safer, more consistent, and less error prone
 * than strncat. It guarantees that the destination string is always null-terminated.
 * 
 * BSD extension: guaranteed null-terminated string concatenation
 */

#include <string.h>

/**
 * @brief Size-bounded string concatenation with guaranteed null termination
 * @param dst Destination buffer
 * @param src Source string to append
 * @param size Size of destination buffer
 * @return Length of the string that would have been created
 * 
 * This function appends src to dst, always null-terminating the result.
 * It copies at most size-strlen(dst)-1 characters, ensuring the result
 * fits in the buffer. The return value is the length the string would
 * have been if size had been sufficiently large.
 * 
 * Unlike strncat, this function:
 * - Always null-terminates the destination
 * - Returns the total length that would have been created
 * - Provides a way to detect truncation
 * - Is safer for buffer management
 */
size_t strlcat(char *dst, const char *src, size_t size)
{
    size_t dst_len, src_len, copy_len;
    
    if (dst == NULL || src == NULL) {
        return 0;
    }
    
    dst_len = strlen(dst);
    src_len = strlen(src);
    
    if (size <= dst_len) {
        /* Destination buffer is full, return total length that would be needed */
        return size + src_len;
    }
    
    /* Calculate how many characters we can copy */
    copy_len = size - dst_len - 1;
    
    if (src_len < copy_len) {
        copy_len = src_len;
    }
    
    /* Copy the source string and null-terminate */
    memcpy(dst + dst_len, src, copy_len);
    dst[dst_len + copy_len] = '\0';
    
    /* Return the total length that would have been created */
    return dst_len + src_len;
}
