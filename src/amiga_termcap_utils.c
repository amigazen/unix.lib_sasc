/*
 * Amiga-Specific Termcap Implementation
 * Utility functions and memory management
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "amiga_termcap_private.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global error state */
static int g_termcap_error = AMIGA_TERMCAP_ERROR_NONE;

/*
 * amiga_termcap_malloc - memory allocation wrapper
 * 
 * This function provides memory allocation with error handling.
 */
void *
amiga_termcap_malloc(size_t size)
{
    void *ptr;
    
    if (size == 0) {
        return NULL;
    }
    
    ptr = malloc(size);
    if (!ptr) {
        g_termcap_error = AMIGA_TERMCAP_ERROR_MEMORY;
    }
    
    return ptr;
}

/*
 * amiga_termcap_free - memory deallocation wrapper
 * 
 * This function provides memory deallocation with error handling.
 */
void
amiga_termcap_free(void *ptr)
{
    if (ptr) {
        free(ptr);
    }
}

/*
 * amiga_termcap_realloc - memory reallocation wrapper
 * 
 * This function provides memory reallocation with error handling.
 */
void *
amiga_termcap_realloc(void *ptr, size_t size)
{
    void *new_ptr;
    
    if (size == 0) {
        amiga_termcap_free(ptr);
        return NULL;
    }
    
    new_ptr = realloc(ptr, size);
    if (!new_ptr && size > 0) {
        g_termcap_error = AMIGA_TERMCAP_ERROR_MEMORY;
    }
    
    return new_ptr;
}

/*
 * amiga_termcap_strlen - string length function
 * 
 * This function provides string length calculation.
 */
int
amiga_termcap_strlen(const char *str)
{
    int len = 0;
    
    if (!str) {
        return 0;
    }
    
    while (*str++) {
        len++;
    }
    
    return len;
}

/*
 * amiga_termcap_strdup - string duplication function
 * 
 * This function provides string duplication with error handling.
 */
char *
amiga_termcap_strdup(const char *str)
{
    char *dup;
    int len;
    
    if (!str) {
        return NULL;
    }
    
    len = amiga_termcap_strlen(str);
    dup = amiga_termcap_malloc(len + 1);
    if (!dup) {
        return NULL;
    }
    
    strcpy(dup, str);
    return dup;
}

/*
 * amiga_termcap_strcmp - string comparison function
 * 
 * This function provides string comparison.
 */
int
amiga_termcap_strcmp(const char *s1, const char *s2)
{
    if (!s1 && !s2) {
        return 0;
    }
    if (!s1) {
        return -1;
    }
    if (!s2) {
        return 1;
    }
    
    while (*s1 && *s2 && *s1 == *s2) {
        s1++;
        s2++;
    }
    
    return *s1 - *s2;
}

/*
 * amiga_termcap_strchr - character search function
 * 
 * This function searches for a character in a string.
 */
char *
amiga_termcap_strchr(const char *str, int c)
{
    if (!str) {
        return NULL;
    }
    
    while (*str) {
        if (*str == c) {
            return (char *)str;
        }
        str++;
    }
    
    if (c == '\0') {
        return (char *)str;
    }
    
    return NULL;
}

/*
 * amiga_termcap_strstr - substring search function
 * 
 * This function searches for a substring in a string.
 */
char *
amiga_termcap_strstr(const char *haystack, const char *needle)
{
    const char *h, *n;
    int needle_len;
    
    if (!haystack || !needle) {
        return NULL;
    }
    
    needle_len = amiga_termcap_strlen(needle);
    if (needle_len == 0) {
        return (char *)haystack;
    }
    
    while (*haystack) {
        h = haystack;
        n = needle;
        
        while (*h && *n && *h == *n) {
            h++;
            n++;
        }
        
        if (*n == '\0') {
            return (char *)haystack;
        }
        
        haystack++;
    }
    
    return NULL;
}

/*
 * amiga_termcap_set_error - set error state
 * 
 * This function sets the global error state.
 */
int
amiga_termcap_set_error(int error)
{
    g_termcap_error = error;
    return error;
}

/*
 * amiga_termcap_get_error - get error state
 * 
 * This function gets the global error state.
 */
int
amiga_termcap_get_error(void)
{
    return g_termcap_error;
}

/*
 * amiga_termcap_expand_string - expand escape sequences in string
 * 
 * This function expands escape sequences in a termcap string.
 */
char *
amiga_termcap_expand_string(const char *str, int *len)
{
    char *result;
    char *p;
    int result_len, i;
    
    if (!str || !len) {
        return NULL;
    }
    
    /* Calculate length needed */
    result_len = 0;
    p = (char *)str;
    while (*p) {
        if (*p == '\\') {
            p++;
            if (*p) {
                result_len++;
                p++;
            }
        } else {
            result_len++;
            p++;
        }
    }
    
    /* Allocate result buffer */
    result = amiga_termcap_malloc(result_len + 1);
    if (!result) {
        return NULL;
    }
    
    /* Expand escape sequences */
    i = 0;
    p = (char *)str;
    while (*p && i < result_len) {
        if (*p == '\\') {
            p++;
            if (*p) {
                switch (*p) {
                case 'n':
                    result[i++] = '\n';
                    break;
                case 'r':
                    result[i++] = '\r';
                    break;
                case 't':
                    result[i++] = '\t';
                    break;
                case 'b':
                    result[i++] = '\b';
                    break;
                case 'f':
                    result[i++] = '\f';
                    break;
                case '\\':
                    result[i++] = '\\';
                    break;
                case '^':
                    p++;
                    if (*p) {
                        result[i++] = *p - '@';
                    }
                    break;
                default:
                    if (*p >= '0' && *p <= '7') {
                        /* Octal escape */
                        int oct = *p - '0';
                        p++;
                        if (*p >= '0' && *p <= '7') {
                            oct = oct * 8 + (*p - '0');
                            p++;
                            if (*p >= '0' && *p <= '7') {
                                oct = oct * 8 + (*p - '0');
                            } else {
                                p--;
                            }
                        } else {
                            p--;
                        }
                        result[i++] = (char)oct;
                    } else {
                        result[i++] = *p;
                    }
                    break;
                }
                p++;
            }
        } else {
            result[i++] = *p++;
        }
    }
    
    result[i] = '\0';
    *len = i;
    
    return result;
}

/*
 * amiga_termcap_handle_escape - handle escape sequence
 * 
 * This function handles a single escape sequence.
 */
int
amiga_termcap_handle_escape(const char **str, char *output, int maxlen)
{
    const char *p;
    int len = 0;
    
    if (!str || !*str || !output || maxlen <= 0) {
        return 0;
    }
    
    p = *str;
    if (*p != '\\') {
        return 0;
    }
    
    p++;
    if (!*p) {
        return 0;
    }
    
    switch (*p) {
    case 'n':
        if (len < maxlen) {
            output[len++] = '\n';
        }
        break;
    case 'r':
        if (len < maxlen) {
            output[len++] = '\r';
        }
        break;
    case 't':
        if (len < maxlen) {
            output[len++] = '\t';
        }
        break;
    case 'b':
        if (len < maxlen) {
            output[len++] = '\b';
        }
        break;
    case 'f':
        if (len < maxlen) {
            output[len++] = '\f';
        }
        break;
    case '\\':
        if (len < maxlen) {
            output[len++] = '\\';
        }
        break;
    case '^':
        p++;
        if (*p && len < maxlen) {
            output[len++] = *p - '@';
        }
        break;
    default:
        if (*p >= '0' && *p <= '7') {
            /* Octal escape */
            int oct = *p - '0';
            p++;
            if (*p >= '0' && *p <= '7') {
                oct = oct * 8 + (*p - '0');
                p++;
                if (*p >= '0' && *p <= '7') {
                    oct = oct * 8 + (*p - '0');
                } else {
                    p--;
                }
            } else {
                p--;
            }
            if (len < maxlen) {
                output[len++] = (char)oct;
            }
        } else {
            if (len < maxlen) {
                output[len++] = *p;
            }
        }
        break;
    }
    
    *str = p + 1;
    return len;
}
