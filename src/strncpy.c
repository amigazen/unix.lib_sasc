/*
 * strncpy.c - copy a string with length limit using AmigaOS native function
 * 
 * This implementation uses the AmigaOS dos.library Strncpy() function
 * which provides memory-safe string copying with proper truncation handling.
 * 
 * POSIX compliant: copies up to n characters, always null-terminates
 */

#include <proto/dos.h>
#include <string.h>

/**
 * @brief Copy a string with length limit using AmigaOS native function
 * @param dest Destination buffer
 * @param src Source string
 * @param n Maximum number of characters to copy
 * @return Pointer to destination string
 * 
 * This function uses AmigaOS dos.library Strncpy() which:
 * - Always null-terminates the destination (if n > 0)
 * - Handles truncation gracefully
 * - Returns pointer to destination for chaining
 * - Provides better performance than manual implementation
 */
char *strncpy(char *dest, const char *src, size_t n)
{
    if (dest == NULL || src == NULL) {
        return dest;
    }
    
    /* Use AmigaOS native Strncpy function for optimal performance */
    Strncpy((UBYTE *)dest, (const UBYTE *)src, (ULONG)n);
    
    return dest;
}
