/*
 * t_goto_enhanced.c - Enhanced cursor positioning and output for Amiga
 *
 * Copyright (c) 2025 amigazen project
 *
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * This file implements enhanced cursor positioning and output functions
 * specifically designed for Amiga console capabilities.
 */

#include "amiga_termcap_private.h"
#include "termcap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#define MAX_CURSOR_BUFFER 128

/*
 * Amiga-specific delay timing table
 * Based on typical Amiga console output speeds
 */
static const short amiga_delay_table[] = {
    0,    /* 0 - no delay */
    100,  /* 1 - very slow */
    50,   /* 2 - slow */
    25,   /* 3 - medium */
    10,   /* 4 - fast */
    5,    /* 5 - very fast */
    2,    /* 6 - ultra fast */
    1     /* 7 - maximum speed */
};

/*
 * Parse delay specification from termcap string
 * 
 * Handles formats like "10", "5.5", "10*" where:
 * - "10" = 10ms delay
 * - "5.5" = 5.5ms delay  
 * - "10*" = 10ms * affected_lines
 */
static int
parse_delay(const char **ptr, int affected_lines)
{
    int delay = 0;
    int fractional = 0;
    
    if (!ptr || !*ptr) {
        return 0;
    }
    
    /* Parse integer part */
    while (isdigit((unsigned char)**ptr)) {
        delay = delay * 10 + (**ptr - '0');
        (*ptr)++;
    }
    
    /* Parse fractional part */
    if (**ptr == '.') {
        (*ptr)++;
        if (isdigit((unsigned char)**ptr)) {
            fractional = **ptr - '0';
            (*ptr)++;
            /* Skip additional fractional digits */
            while (isdigit((unsigned char)**ptr)) {
                (*ptr)++;
            }
        }
    }
    
    /* Apply fractional part */
    delay = delay * 10 + fractional;
    
    /* Handle multiplication by affected lines */
    if (**ptr == '*') {
        (*ptr)++;
        delay *= affected_lines;
    }
    
    return delay;
}

/*
 * Enhanced t_goto - Generate cursor positioning sequence
 * 
 * This function generates cursor positioning sequences using the cm capability.
 * It supports the standard %d format specifier for Amiga console.
 * 
 * For Amiga, we use the simple format: \033[%d;%dH
 * where the first %d is the line and the second %d is the column.
 */
int
t_goto(struct tinfo *info, const char *cm_cap, int destcol, int destline, char *buffer, size_t limit)
{
    const char *cp;
    char *dp;
    int line, col;
    
    if (!cm_cap || !buffer || limit < 16) {
        errno = EINVAL;
        return -1;
    }
    
    /* Set global variables for compatibility */
    if (info) {
        if (!UP && info->up) {
            UP = info->up;
        }
        if (!BC && info->bc) {
            BC = info->bc;
        }
    }
    
    /* For Amiga, we expect cm capability to be \033[%d;%dH */
    cp = cm_cap;
    dp = buffer;
    
    /* Copy the sequence up to the first %d */
    while (*cp && *cp != '%') {
        if (dp >= buffer + limit - 1) {
            errno = E2BIG;
            return -1;
        }
        *dp++ = *cp++;
    }
    
    if (*cp != '%' || *(cp + 1) != 'd') {
        errno = EINVAL;
        return -1;
    }
    
    /* Skip %d */
    cp += 2;
    
    /* Add line number */
    line = destline + 1;  /* Amiga uses 1-based coordinates */
    if (line < 10) {
        *dp++ = '0' + line;
    } else if (line < 100) {
        *dp++ = '0' + (line / 10);
        *dp++ = '0' + (line % 10);
    } else {
        /* Handle large line numbers */
        if (line >= 1000) {
            errno = E2BIG;
            return -1;
        }
        *dp++ = '0' + (line / 100);
        *dp++ = '0' + ((line % 100) / 10);
        *dp++ = '0' + (line % 10);
    }
    
    /* Copy the separator */
    while (*cp && *cp != '%') {
        if (dp >= buffer + limit - 1) {
            errno = E2BIG;
            return -1;
        }
        *dp++ = *cp++;
    }
    
    if (*cp != '%' || *(cp + 1) != 'd') {
        errno = EINVAL;
        return -1;
    }
    
    /* Skip %d */
    cp += 2;
    
    /* Add column number */
    col = destcol + 1;  /* AmigaOS uses 1-based coordinates */
    if (col < 10) {
        *dp++ = '0' + col;
    } else if (col < 100) {
        *dp++ = '0' + (col / 10);
        *dp++ = '0' + (col % 10);
    } else {
        /* Handle large column numbers */
        if (col >= 1000) {
            errno = E2BIG;
            return -1;
        }
        *dp++ = '0' + (col / 100);
        *dp++ = '0' + ((col % 100) / 10);
        *dp++ = '0' + (col % 10);
    }
    
    /* Copy the rest of the sequence */
    while (*cp) {
        if (dp >= buffer + limit - 1) {
            errno = E2BIG;
            return -1;
        }
        *dp++ = *cp++;
    }
    
    *dp = '\0';
    return 0;
}

/*
 * Enhanced t_puts - Output string with proper delay handling
 * 
 * This function outputs a termcap string with appropriate delays
 * for Amiga console characteristics.
 */
int
t_puts(struct tinfo *info, const char *cp, int affcnt, void (*outc)(char, void *), void *args)
{
    int delay = 0;
    char pad_char = '\0';
    const char *ptr;
    
    if (!cp || !outc) {
        errno = EINVAL;
        return -1;
    }
    
    /* Get pad character from termcap entry if available */
    if (info) {
        char pad_buffer[4];
        char *pad_ptr = pad_buffer;
        size_t pad_limit = sizeof(pad_buffer);
        char *pc = t_getstr(info, "pc", &pad_ptr, &pad_limit);
        if (pc) {
            pad_char = *pc;
        }
    }
    
    /* Parse delay specification at the beginning of the string */
    ptr = cp;
    delay = parse_delay(&ptr, affcnt);
    
    /* Output the actual string */
    while (*ptr) {
        (*outc)(*ptr++, args);
    }
    
    /* Apply delay if specified */
    if (delay > 0) {
        /* Convert delay to appropriate units for Amiga */
        int delay_units = delay / 10;  /* Convert to 10ms units */
        
        /* Limit delay to reasonable values for Amiga */
        if (delay_units > 100) {
            delay_units = 100;  /* Maximum 1 second delay */
        }
        
        /* Output pad characters for delay */
        while (delay_units > 0) {
            (*outc)(pad_char, args);
            delay_units--;
        }
    }
    
    return 0;
}
