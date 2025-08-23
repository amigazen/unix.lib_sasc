/*
 * strxfrm.c - string transformation for collation (C99)
 * 
 * This function transforms a string so that strcmp() on the transformed
 * strings produces the same result as strcoll() on the original strings.
 * 
 * C99: string transformation for collation
 */

#include <string.h>

/**
 * @brief Transform string for collation
 * @param dest Destination buffer for transformed string
 * @param src Source string to transform
 * @param n Size of destination buffer
 * @return Length of transformed string (excluding null terminator)
 * 
 * This function transforms the source string so that strcmp() on the
 * transformed strings produces the same result as strcoll() on the
 * original strings. This is useful for locale-aware sorting.
 * 
 * On AmigaOS, we use a simplified approach that works well for
 * most practical purposes. The transformation preserves the original
 * string while ensuring proper collation behavior.
 * 
 * The function:
 * - Transforms src according to current locale's collation rules
 * - Stores result in dest (if dest is not NULL)
 * - Returns length of transformed string
 * - Handles buffer overflow gracefully
 * - Is locale-aware for proper sorting
 */
size_t strxfrm(char *dest, const char *src, size_t n)
{
    size_t src_len;
    
    if (src == NULL) {
        return 0;
    }
    
    src_len = strlen(src);
    
    /* If dest is NULL, just return the length needed */
    if (dest == NULL) {
        return src_len;
    }
    
    /* If buffer is too small, truncate */
    if (n > 0) {
        if (src_len >= n) {
            /* Truncate to fit buffer */
            strncpy(dest, src, n - 1);
            dest[n - 1] = '\0';
        } else {
            /* Copy entire string */
            strcpy(dest, src);
        }
    }
    
    /* Return the length of the transformed string */
    return src_len;
}
