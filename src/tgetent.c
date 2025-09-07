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

/* External database reference */
extern struct termcap_db *g_termcap_db;

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
tgetent(char *bp, const char *name)
{
    char *entry;
    int result;
    size_t len;
    
    if (!bp || !name) {
        return -1;
    }
    
    
    /* Load database if not already loaded */
    if (!g_termcap_db) {
        if (amiga_termcap_load_embedded_db() != 0) {
            return -1;
        }
    }
    
    /* Try to get entry from database */
    entry = amiga_termcap_find_entry(name);
    if (!entry) {
        return -1;
    }
    
    /* Copy entry to buffer - use full length to preserve all capabilities */
    len = amiga_termcap_strlen(entry);
    strncpy(bp, entry, len);
    bp[len] = '\0';
    
    /* Store global buffer pointer */
    g_termcap_buffer = bp;
    
    /* Initialize global variables */
    PC = '\0';
    BC = NULL;
    UP = NULL;
    
    /* Global variables will be set by individual capability functions when called */
    
    amiga_termcap_free(entry);
    
    return 1;
}

/*
 * tgetstr - get a string capability from termcap entry
 * 
 * This function extracts a string capability from the current termcap entry.
 */
char *
tgetstr(const char *id, char **area)
{
    char *result;
    
    if (!g_termcap_buffer || !id || !area) {
        return NULL;
    }
    
    result = cgetstr(g_termcap_buffer, id, area);
    
    /* Set global variables for common capabilities */
    if (result) {
        if (amiga_termcap_strcmp(id, "bc") == 0) {
            BC = result;
        } else if (amiga_termcap_strcmp(id, "up") == 0) {
            UP = result;
        }
    }
    
    return result;
}

/*
 * tgetflag - check if a boolean capability is present
 * 
 * This function checks if a boolean capability is present in the current
 * termcap entry.
 */
int
tgetflag(const char *id)
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
tgetnum(const char *id)
{
    long num;
    
    if (!g_termcap_buffer || !id) {
        return -1;
    }
    
    if (cgetnum(g_termcap_buffer, id, &num) != 0) {
        return -1;
    }
    
    /* Set global variables for common capabilities */
    if (amiga_termcap_strcmp(id, "pc") == 0) {
        PC = (char)num;
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
tgoto(const char *cm, int destcol, int destline)
{
    static char result[64];
    char *p, *q;
    int i, j;
    int oncol = 0;  /* Track whether we're on column or line */
    int which;
    
    if (!cm) {
        return NULL;
    }
    
    
    /* Copy the capability string and substitute coordinates */
    p = (char *)cm;
    q = result;
    i = 0;
    which = destline;  /* Start with line (0-based) */
    
    while (*p && i < sizeof(result) - 1) {
        if (*p == '%') {
            p++;
            switch (*p) {
            case 'd':  /* Decimal number */
                sprintf(q, "%d", which);
                while (*q) {
                    q++;
                    i++;
                }
                /* Switch between line and column */
                oncol = 1 - oncol;
                which = oncol ? destcol : destline;
                break;
            case '2':  /* Two-digit decimal */
                sprintf(q, "%02d", which);
                while (*q) {
                    q++;
                    i++;
                }
                /* Switch between line and column */
                oncol = 1 - oncol;
                which = oncol ? destcol : destline;
                break;
            case '3':  /* Three-digit decimal */
                sprintf(q, "%03d", which);
                while (*q) {
                    q++;
                    i++;
                }
                /* Switch between line and column */
                oncol = 1 - oncol;
                which = oncol ? destcol : destline;
                break;
            case 'i':  /* Increment coordinates */
                destcol++;
                destline++;
                /* Update current which value */
                which = oncol ? destcol : destline;
                break;
            case 'r':  /* Reverse coordinates */
                j = destcol;
                destcol = destline;
                destline = j;
                /* Update current which value */
                which = oncol ? destcol : destline;
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
int
tputs(const char *cp, int affcnt, int (*outc)(int))
{
    int len, pad;
    
    if (!cp || !outc) {
        return -1;
    }
    
    
    /* Output the string */
    while (*cp) {
        if (outc(*cp++) == EOF) {
            return -1;
        }
    }
    
    /* Calculate padding */
    len = amiga_termcap_strlen(cp);
    if (PC && len > 0) {
        pad = (len * affcnt) / 10;
        while (pad-- > 0) {
            if (outc(PC) == EOF) {
                return -1;
            }
        }
    }
    
    return 0;
}
