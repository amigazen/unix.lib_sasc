/*
 * strndup.c - length-limited string duplication (POSIX.1-2008)
 * 
 * This function returns a pointer to a new string which is a duplicate
 * of the string s, but copies at most n characters.
 * 
 * POSIX.1-2008: length-limited string duplication
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>

/**
 * @brief Length-limited string duplication
 * @param s Source string to duplicate
 * @param n Maximum number of characters to copy
 * @return Pointer to new string, or NULL on failure
 * 
 * This function creates a new string containing at most n characters
 * from the source string. The new string is always null-terminated.
 * 
 * Memory for the new string is obtained with malloc(), and can be
 * freed with free(). If s is NULL, the function returns NULL.
 */
char *strndup(const char *s, size_t n)
{
    char *dup;
    size_t len, copy_len;
    
    if (s == NULL) {
        errno = EINVAL;
        return NULL;
    }
    
    /* Calculate actual length to copy */
    len = strlen(s);
    copy_len = (len < n) ? len : n;
    
    /* Allocate space for string + null terminator */
    dup = malloc(copy_len + 1);
    if (dup == NULL) {
        return NULL;  /* errno already set by malloc */
    }
    
    /* Copy the string and null-terminate */
    memcpy(dup, s, copy_len);
    dup[copy_len] = '\0';
    
    return dup;
}
