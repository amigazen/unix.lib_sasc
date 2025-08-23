/*
 * strnicmp.c - length-limited case-insensitive string comparison using AmigaOS native function
 * 
 * This implementation uses the AmigaOS dos.library Strnicmp() function
 * which provides locale-aware case-insensitive string comparison with length limit.
 * 
 * POSIX compliant: length-limited case-insensitive string comparison
 */

#include <proto/dos.h>
#include <string.h>

/**
 * @brief Length-limited case-insensitive string comparison using AmigaOS native function
 * @param s1 First string to compare
 * @param s2 Second string to compare
 * @param n Maximum number of characters to compare
 * @return <0 if s1 < s2, 0 if s1 == s2, >0 if s1 > s2
 * 
 * This function uses AmigaOS dos.library Strnicmp() which:
 * - Provides locale-aware case conversion
 * - Handles different language settings automatically
 * - Limits comparison to specified number of characters
 * - Returns standard comparison result values
 * - Provides better performance than manual implementation
 */
int strnicmp(const char *s1, const char *s2, size_t n)
{
    if (s1 == NULL || s2 == NULL) {
        if (s1 == s2) return 0;
        return (s1 == NULL) ? -1 : 1;
    }
    
    /* Use AmigaOS native Strnicmp function for optimal performance */
    return Strnicmp((STRPTR)s1, (STRPTR)s2, (LONG)n);
}
