/*
 * strtok_r.c - reentrant string tokenization (POSIX.1-2001)
 * 
 * This function is a reentrant version of strtok that uses a pointer
 * to save its state instead of static variables, making it thread-safe.
 * 
 * POSIX.1-2001: reentrant string tokenization
 */

#include <string.h>

/**
 * @brief Reentrant string tokenization
 * @param str String to tokenize (NULL for subsequent calls)
 * @param delim String of delimiter characters
 * @param saveptr Pointer to save state between calls
 * @return Pointer to next token, or NULL if no more tokens
 * 
 * This function breaks a string into a sequence of tokens. On the first call,
 * str should point to the string to tokenize. On subsequent calls, str should
 * be NULL and saveptr should point to the same location used in the first call.
 * 
 * The function:
 * - Is reentrant and thread-safe
 * - Modifies the original string
 * - Returns NULL when no more tokens are found
 * - Skips leading delimiters
 * - Handles consecutive delimiters as single separators
 */
char *strtok_r(char *str, const char *delim, char **saveptr)
{
    char *token, *end;
    
    if (delim == NULL || saveptr == NULL) {
        return NULL;
    }
    
    /* If str is NULL, use the saved pointer */
    if (str == NULL) {
        str = *saveptr;
        if (str == NULL) {
            return NULL;  /* No more tokens */
        }
    }
    
    /* Skip leading delimiters */
    while (*str && strchr(delim, *str)) {
        str++;
    }
    
    if (*str == '\0') {
        *saveptr = NULL;
        return NULL;  /* No more tokens */
    }
    
    token = str;
    
    /* Find the next delimiter */
    while (*str && !strchr(delim, *str)) {
        str++;
    }
    
    if (*str == '\0') {
        /* This is the last token */
        *saveptr = NULL;
    } else {
        /* Replace delimiter with null terminator and save position */
        *str = '\0';
        *saveptr = str + 1;
    }
    
    return token;
}
