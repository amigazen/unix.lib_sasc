/*
 * strnlen.c - length-limited string length (POSIX.1-2008)
 * 
 * This function returns the length of a string, but never more than maxlen.
 * It's useful for checking string length without risking buffer overruns.
 * 
 * POSIX.1-2008: length-limited string length
 */

#include <string.h>

/**
 * @brief Length-limited string length
 * @param s String to measure
 * @param maxlen Maximum length to check
 * @return Length of string, or maxlen if string is longer
 * 
 * This function returns the length of the string s, but never more than maxlen.
 * It's useful for:
 * - Checking string length without buffer overruns
 * - Validating input strings
 * - Safe string processing
 * 
 * The function stops counting at the first null terminator or at maxlen,
 * whichever comes first.
 */
size_t strnlen(const char *s, size_t maxlen)
{
    size_t len = 0;
    
    if (s == NULL) {
        return 0;
    }
    
    while (len < maxlen && s[len] != '\0') {
        len++;
    }
    
    return len;
}
