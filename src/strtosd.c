/*
 * SPDX-License-Identifier: BSD-2-Clause
 * strtosd.c - parse a scaled double from an ascii string
 *
 * Based on PDC stdlib strtosd.c by J.A. Lydiatt
 * Copyright (C) 2025 by amigazen project
 */

#ifndef _WITH_SCLIB

#include <stddef.h>
#include <ctype.h>

double strtosd(char *string, char **ptr, double base)
{
    char *strptr = string;
    double retval = 0.0;
    double sign;
    double obase = base;
    int i = 0;
    char c;

    obase = base;

    /* Skip leading whitespace */
    while (isspace(c = *strptr))
        strptr++;

    /* Handle sign */
    if (c == '-') {
        sign = -1.0;
        c = *(++strptr);
    } else {
        sign = 1.0;
    }

    if (c == '+')
        c = *(++strptr);

    /* Parse digits */
    while ((c = *strptr) != 0) {
        if (isdigit(c)) {
            i++;
            strptr++;
            if (obase > 1)
                retval = (retval * base) + (c - '0');
            else {
                retval += base * (c - '0');
                base *= obase;
            }
        } else {
            break;
        }
    }

    /* Set end pointer */
    if (ptr != NULL) {
        if (i > 0)
            *ptr = strptr;
        else
            *ptr = string;
    }

    return retval * sign;
}

#else
/* Empty - function provided by SAS/C sc.lib */
#endif
