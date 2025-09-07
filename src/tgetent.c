/*
 * Amiga-Specific Termcap Implementation
 * Standard termcap interface functions
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "amiga_termcap_private.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global variables for compatibility */
char PC = '\0';                   /* Pad character */
char *BC = NULL;                  /* Backspace character */
char *UP = NULL;                  /* Up cursor movement */
short ospeed = 9600;              /* Output speed */

/* Global termcap buffer */
static char *g_termcap_buffer = NULL;

/*
 * tgetent - get a termcap entry for a terminal
 * 
 * This is the main entry point for the standard termcap interface.
 * It loads a termcap entry for the specified terminal name.
 */
int
tgetent(char *bp, char *name)
{
    char *entry;
    int result;
    size_t len;
    
    if (!bp || !name) {
        return -1;
    }
    
    
    /* Try to get entry from database */
    result = cgetent(&entry, NULL, name);
    if (result != 0) {
        return -1;
    }
    
    /* Copy entry to buffer, truncating if necessary */
    len = amiga_termcap_strlen(entry);
    if (len >= 1024) {
        len = 1023;
    }
    
    strncpy(bp, entry, len);
    bp[len] = '\0';
    
    /* Store global buffer pointer */
    g_termcap_buffer = bp;
    
    /* Initialize global variables */
    PC = '\0';
    BC = NULL;
    UP = NULL;
    
    /* Extract common capabilities */
    {
        char *area = bp + len + 1;
        char *bc_str, *up_str;
        
        /* Get backspace capability */
        bc_str = tgetstr("bc", &area);
        if (bc_str) {
            BC = bc_str;
        }
        
        /* Get up cursor capability */
        up_str = tgetstr("up", &area);
        if (up_str) {
            UP = up_str;
        }
        
        /* Get pad character */
        {
            long pad_char;
            if (cgetnum(bp, "pc", &pad_char) == 0) {
                PC = (char)pad_char;
            }
        }
    }
    
    amiga_termcap_free(entry);
    
    return 1;
}

/*
 * tgetstr - get a string capability from termcap entry
 * 
 * This function extracts a string capability from the current termcap entry.
 */
char *
tgetstr(char *id, char **area)
{
    if (!g_termcap_buffer || !id || !area) {
        return NULL;
    }
    
    
    return cgetstr(g_termcap_buffer, id, area);
}

/*
 * tgetflag - check if a boolean capability is present
 * 
 * This function checks if a boolean capability is present in the current
 * termcap entry.
 */
int
tgetflag(char *id)
{
    if (!g_termcap_buffer || !id) {
        return 0;
    }
    
    
    return cgetflag(g_termcap_buffer, id);
}

/*
 * tgetnum - get a numeric capability from termcap entry
 * 
 * This function extracts a numeric capability from the current termcap entry.
 */
int
tgetnum(char *id)
{
    long num;
    
    if (!g_termcap_buffer || !id) {
        return -1;
    }
    
    
    if (cgetnum(g_termcap_buffer, id, &num) != 0) {
        return -1;
    }
    
    return (int)num;
}

/*
 * tgoto - generate cursor positioning string
 * 
 * This function generates a cursor positioning string with the given
 * coordinates, using the cursor motion capability.
 */
char *
tgoto(char *cm, int destcol, int destline)
{
    static char result[64];
    char *p, *q;
    int i, j;
    
    if (!cm) {
        return NULL;
    }
    
    
    /* Copy the capability string and substitute coordinates */
    p = (char *)cm;
    q = result;
    i = 0;
    
    while (*p && i < sizeof(result) - 1) {
        if (*p == '%') {
            p++;
            switch (*p) {
            case 'd':  /* Decimal number */
                sprintf(q, "%d", destline);
                while (*q) {
                    q++;
                    i++;
                }
                break;
            case '2':  /* Two-digit decimal */
                sprintf(q, "%02d", destline);
                while (*q) {
                    q++;
                    i++;
                }
                break;
            case '3':  /* Three-digit decimal */
                sprintf(q, "%03d", destline);
                while (*q) {
                    q++;
                    i++;
                }
                break;
            case 'i':  /* Increment coordinates */
                destcol++;
                destline++;
                break;
            case 'r':  /* Reverse coordinates */
                j = destcol;
                destcol = destline;
                destline = j;
                break;
            case '%':  /* Literal % */
                *q++ = '%';
                i++;
                break;
            default:
                /* Unknown escape, copy as-is */
                *q++ = '%';
                *q++ = *p;
                i += 2;
                break;
            }
            p++;
        } else {
            *q++ = *p++;
            i++;
        }
    }
    
    *q = '\0';
    
    return result;
}

/*
 * tputs - output a string with padding
 * 
 * This function outputs a string with appropriate padding based on
 * the terminal's padding requirements.
 */
void
tputs(char *cp, int affcnt, int (*outc)(int))
{
    int len, pad;
    
    if (!cp || !outc) {
        return;
    }
    
    
    /* Output the string */
    while (*cp) {
        outc(*cp++);
    }
    
    /* Calculate padding */
    len = amiga_termcap_strlen(cp);
    if (PC && len > 0) {
        pad = (len * affcnt) / 10;
        while (pad-- > 0) {
            outc(PC);
        }
    }
}
