/*
 * memccpy.c - copy memory until character found (C99)
 * 
 * This function copies bytes from source to destination until a specific
 * character is found, or until n bytes have been copied, whichever comes first.
 * 
 * C99: copy memory until character found
 */

#include <string.h>

/**
 * @brief Copy memory until character found
 * @param dest Destination buffer
 * @param src Source buffer
 * @param c Character to search for
 * @param n Maximum number of bytes to copy
 * @return Pointer to byte after c in dest, or NULL if c not found
 * 
 * This function copies bytes from src to dest until either:
 * - The character c is found (copied and function returns pointer to next byte)
 * - n bytes have been copied (function returns NULL)
 * 
 * The function:
 * - Always copies at most n bytes
 * - Returns pointer to byte after c if found
 * - Returns NULL if c is not found within n bytes
 * - Handles overlapping memory regions correctly
 * - Is useful for parsing null-terminated strings in binary data
 */
void *memccpy(void *dest, const void *src, int c, size_t n)
{
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;
    unsigned char uc = (unsigned char)c;
    size_t i;
    
    if (dest == NULL || src == NULL || n == 0) {
        return NULL;
    }
    
    /* Copy bytes until we find the character or reach n */
    for (i = 0; i < n; i++) {
        d[i] = s[i];
        if (s[i] == uc) {
            /* Found the character, return pointer to next byte */
            return &d[i + 1];
        }
    }
    
    /* Character not found within n bytes */
    return NULL;
}
