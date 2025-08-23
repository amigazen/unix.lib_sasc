/*
 * stricmp.c - case-insensitive string comparison using AmigaOS native function
 * 
 * This implementation uses the AmigaOS dos.library Stricmp() function
 * which provides locale-aware case-insensitive string comparison.
 * 
 * POSIX compliant: case-insensitive string comparison
 */

#include <proto/dos.h>
#include <string.h>

/**
 * @brief Case-insensitive string comparison using AmigaOS native function
 * @param s1 First string to compare
 * @param s2 Second string to compare
 * @return <0 if s1 < s2, 0 if s1 == s2, >0 if s1 > s2
 * 
 * This function uses AmigaOS dos.library Stricmp() which:
 * - Provides locale-aware case conversion
 * - Handles different language settings automatically
 * - Returns standard comparison result values
 * - Provides better performance than manual implementation
 */
int stricmp(const char *s1, const char *s2)
{
    if (s1 == NULL || s2 == NULL) {
        if (s1 == s2) return 0;
        return (s1 == NULL) ? -1 : 1;
    }
    
    /* Use AmigaOS native Stricmp function for optimal performance */
    return Stricmp((STRPTR)s1, (STRPTR)s2);
}
