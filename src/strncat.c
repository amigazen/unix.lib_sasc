/*
 * strncat.c - concatenate strings with length limit using AmigaOS native function
 * 
 * This implementation uses the AmigaOS dos.library Strncat() function
 * which provides memory-safe string concatenation with proper truncation handling.
 * 
 * POSIX compliant: appends up to n characters, always null-terminates
 */

#include <proto/dos.h>
#include <string.h>

/**
 * @brief Concatenate strings with length limit using AmigaOS native function
 * @param dest Destination buffer
 * @param src Source string to append
 * @param n Maximum number of characters to append
 * @return Pointer to destination string
 * 
 * This function uses AmigaOS dos.library Strncat() which:
 * - Always null-terminates the destination
 * - Handles truncation gracefully
 * - Returns pointer to destination for chaining
 * - Provides better performance than manual implementation
 */
char *strncat(char *dest, const char *src, size_t n)
{
    if (dest == NULL || src == NULL) {
        return dest;
    }
    
    /* Use AmigaOS native Strncat function for optimal performance */
    Strncat((UBYTE *)dest, (const UBYTE *)src, (ULONG)n);
    
    return dest;
}
