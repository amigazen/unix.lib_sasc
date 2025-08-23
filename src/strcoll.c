/*
 * strcoll.c - locale-aware string comparison (POSIX)
 * 
 * This function compares strings according to the current locale's collation rules.
 * For AmigaOS, we use the native Stricmp function which handles locale settings.
 * 
 * POSIX: locale-aware string comparison
 */

#include <proto/dos.h>
#include <string.h>

/**
 * @brief Locale-aware string comparison
 * @param s1 First string to compare
 * @param s2 Second string to compare
 * @return <0 if s1 < s2, 0 if s1 == s2, >0 if s1 > s2
 * 
 * This function compares strings according to the current locale's collation rules.
 * On AmigaOS, we use the native Stricmp function which automatically handles:
 * - Locale settings
 * - Language-specific collation
 * - Case sensitivity rules
 * 
 * The function returns the same values as strcmp for consistency.
 */
int strcoll(const char *s1, const char *s2)
{
    if (s1 == NULL || s2 == NULL) {
        if (s1 == s2) return 0;
        return (s1 == NULL) ? -1 : 1;
    }
    
    /* Use AmigaOS native Stricmp for locale-aware comparison */
    return Stricmp((STRPTR)s1, (STRPTR)s2);
}
