/*
 * SPDX-License-Identifier: BSD-2-Clause
 * strtod.c - convert string to double
 *
 * Based on PDC stdlib strtod.c by J.A. Lydiatt
 * Copyright (C) 2025 by amigazen project
 */

#ifndef _WITH_SCLIB

#include <stdlib.h>
#include <stddef.h>
#include <ctype.h>

/* External helper functions for scaled double parsing */
extern double strtosd(char *, char **, double);
extern double strtosud(char *, char **, double);
double strtod(const char *string, char **ptr)
{
    double retval = 0.0;
    double sign = 1.0;
    char *strptr = (char *)string;
    char *scan_index;
    int exp = 0;
    int valid = 0;              /* Advances when a value is established */

    /* Parse integer part */
    retval = strtosd(strptr, &scan_index, 10.0);
    if (strptr != scan_index) {
        valid++;
        strptr = scan_index;
        if (retval < 0.0) {
            sign = -1.0;
            retval *= -1.0;
        }
    } else {
        /* Skip whitespace and handle sign if no integer part */
        while (isspace(*strptr))
            strptr++;
        if (*strptr == '-') {
            sign = -1.0;
            strptr++;
        } else if (*strptr == '+') {
            strptr++;
        }
    }

    /* Parse fractional part */
    if (*strptr == '.') {
        strptr++;
        retval += strtosud(strptr, &scan_index, 0.1);
        if (strptr != scan_index) {
            valid++;
            strptr = scan_index;
        }
    }

    /* Parse exponent */
    if ((valid > 0) && (tolower(*strptr) == 'e')) {
        strptr++;
        exp = (int)strtol(strptr, &scan_index, 10);
        strptr = scan_index;
        if (exp > 0) {
            while (exp-- > 0)
                retval *= 10.0;
        } else if (exp < 0) {
            while (exp++ < 0)
                retval /= 10.0;
        }
    }

    /* Set end pointer */
    if (ptr != NULL) {
        if (valid > 0)
            *ptr = strptr;
        else
            *ptr = (char *)string;
    }

    return retval * sign;
}

#else
/* Empty - function provided by SAS/C sc.lib */
#endif
