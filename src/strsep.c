/*
 * strsep.c - extract token from string (BSD extension)
 * 
 * This function is a safer alternative to strtok that doesn't use static state
 * and can handle empty fields. It's designed to be reentrant and thread-safe.
 * 
 * BSD extension: reentrant string tokenization
 */

#include <string.h>

/**
 * @brief Extract token from string using delimiter
 * @param stringp Pointer to pointer to string to search
 * @param delim String of delimiter characters
 * @return Pointer to token, or NULL if no more tokens
 * 
 * This function locates the first occurrence of any character in delim
 * in the string *stringp, replaces it with a null byte, and updates
 * *stringp to point to the next character after the delimiter.
 * 
 * Unlike strtok, this function:
 * - Is reentrant (no static state)
 * - Can handle empty fields
 * - Is thread-safe
 * - Modifies the original string
 * - Returns NULL when no more tokens are found
 */
char *strsep(char **stringp, const char *delim)
{
    char *token, *end;
    
    if (stringp == NULL || *stringp == NULL || delim == NULL) {
        return NULL;
    }
    
    token = *stringp;
    
    /* Find the first delimiter */
    end = strpbrk(token, delim);
    
    if (end != NULL) {
        /* Replace delimiter with null terminator */
        *end = '\0';
        /* Move pointer to character after delimiter */
        *stringp = end + 1;
    } else {
        /* No delimiter found, this is the last token */
        *stringp = NULL;
    }
    
    return token;
}
