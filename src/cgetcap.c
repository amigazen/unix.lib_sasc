/*
 * Amiga-Specific Termcap Implementation
 * Core capability parsing functions
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "amiga_termcap_private.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * cgetcap - get a capability value from a termcap entry
 * 
 * This function searches for a capability in a termcap entry string.
 * It handles the termcap format with colons as separators and
 * different capability types (boolean, numeric, string).
 */
int
cgetcap(char *buf, const char *cap, int type)
{
    char *p, *q;
    int found;
    
    if (!buf || !cap) {
        return -1;
    }
    
    
    /* Search through the termcap entry */
    p = buf;
    found = 0;
    
    while (*p) {
        /* Skip to next capability */
        while (*p && *p != ':') {
            p++;
        }
        if (*p == ':') {
            p++; /* Skip the colon */
        }
        
        /* Check if this capability matches */
        q = (char *)cap;
        while (*p && *q && *p == *q && *p != ':' && *p != '=' && *p != '#') {
            p++;
            q++;
        }
        
        /* Check if we found a complete match */
        if (*q == '\0' && (*p == ':' || *p == '=' || *p == '#' || *p == '\0')) {
            found = 1;
            break;
        }
        
        /* Skip to next capability */
        while (*p && *p != ':') {
            p++;
        }
    }
    
    if (!found) {
        return -1;
    }
    
    /* Handle different capability types */
    switch (type) {
    case ':':  /* Boolean capability */
        return 1;
        
    case '#':  /* Numeric capability */
        if (*p == '#') {
            p++; /* Skip the # */
            return (int)strtol(p, NULL, 10);
        }
        break;
        
    case '=':  /* String capability */
        if (*p == '=') {
            p++; /* Skip the = */
            return (int)(long)p; /* Return pointer to string value */
        }
        break;
    }
    
    return -1;
}

/*
 * cgetstr - get a string capability value
 * 
 * This function extracts a string capability value from a termcap entry,
 * handling escape sequences and storing the result in the provided area.
 */
char *
cgetstr(char *buf, const char *cap, char **area)
{
    char *p, *q;
    int len, i;
    char *result;
    
    if (!buf || !cap || !area) {
        return NULL;
    }
    
    /* Find the capability */
    p = (char *)cgetcap(buf, cap, '=');
    if (p == (char *)-1) {
        return NULL;
    }
    
    /* Calculate length needed */
    len = 0;
    q = p;
    while (*q && *q != ':') {
        if (*q == '\\') {
            q++;
            if (*q) {
                len++;
                q++;
            }
        } else {
            len++;
            q++;
        }
    }
    
    /* Allocate space for result */
    result = amiga_termcap_malloc(len + 1);
    if (!result) {
        return NULL;
    }
    
    /* Copy and expand escape sequences */
    i = 0;
    q = p;
    while (*q && *q != ':' && i < len) {
        if (*q == '\\') {
            q++;
            if (*q) {
                switch (*q) {
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
                    q++;
                    if (*q) {
                        result[i++] = *q - '@';
                    }
                    break;
                default:
                    if (*q >= '0' && *q <= '7') {
                        /* Octal escape */
                        int oct = *q - '0';
                        q++;
                        if (*q >= '0' && *q <= '7') {
                            oct = oct * 8 + (*q - '0');
                            q++;
                            if (*q >= '0' && *q <= '7') {
                                oct = oct * 8 + (*q - '0');
                            } else {
                                q--;
                            }
                        } else {
                            q--;
                        }
                        result[i++] = (char)oct;
                    } else {
                        result[i++] = *q;
                    }
                    break;
                }
                q++;
            }
        } else {
            result[i++] = *q++;
        }
    }
    
    result[i] = '\0';
    *area = result;
    
    return result;
}

/*
 * cgetustr - get an unexpanded string capability value
 * 
 * This function extracts a string capability value without expanding
 * escape sequences, useful for raw capability strings.
 */
char *
cgetustr(char *buf, const char *cap, char **area)
{
    char *p, *q;
    int len;
    char *result;
    
    if (!buf || !cap || !area) {
        return NULL;
    }
    
    /* Find the capability */
    p = (char *)cgetcap(buf, cap, '=');
    if (p == (char *)-1) {
        return NULL;
    }
    
    /* Calculate length needed */
    len = 0;
    q = p;
    while (*q && *q != ':') {
        len++;
        q++;
    }
    
    /* Allocate space for result */
    result = amiga_termcap_malloc(len + 1);
    if (!result) {
        return NULL;
    }
    
    /* Copy without expansion */
    strncpy(result, p, len);
    result[len] = '\0';
    
    *area = result;
    
    return result;
}

/*
 * cgetnum - get a numeric capability value
 * 
 * This function extracts a numeric capability value from a termcap entry.
 */
int
cgetnum(char *buf, const char *cap, long *num)
{
    int result;
    
    if (!buf || !cap || !num) {
        return -1;
    }
    
    result = cgetcap(buf, cap, '#');
    if (result == -1) {
        return -1;
    }
    
    *num = (long)result;
    
    return 0;
}

/*
 * cgetflag - check if a boolean capability is present
 * 
 * This function checks if a boolean capability is present in a termcap entry.
 */
int
cgetflag(char *buf, const char *cap)
{
    int result;
    
    if (!buf || !cap) {
        return 0;
    }
    
    result = cgetcap(buf, cap, ':');
    if (result == -1) {
        return 0;
    }
    
    return result;
}
