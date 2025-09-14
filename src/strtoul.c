/*
 * SPDX-License-Identifier: BSD-2-Clause
 * strtoul.c - convert string to unsigned long
 *
 * Based on PDC stdlib strtoul.c by J.A. Lydiatt
 * Copyright (C) 2025 by amigazen project
 */

#ifndef _WITH_SCLIB

#include <stddef.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include "amiga.h"
#include <proto/dos.h>

unsigned long strtoul(const char *string, char **ptr, int base)
{
    char *strptr = (char *)string;
    unsigned long retval = 0;
    char c;
    int i = 0;
    int digit;
    long value;
    long chars;

    /* Skip leading whitespace */
    while (isspace(c = *strptr))
        strptr++;

    /* Handle optional sign (ignore '-' for unsigned) */
    if (c == '+')
        c = *(++strptr);

    /* Auto-detect base if base is 0 */
    if (base == 0) {
        if (c == '0') {
            strptr++;
            c = tolower(*strptr);
            if (c == 'x') {
                base = 16;
                strptr++;
                c = *strptr;
            } else {
                base = 8;
            }
        } else {
            base = 10;
        }
    }

    /* Validate base */
    if (base < 2 || base > 36) {
        errno = EINVAL;
        if (ptr != NULL) *ptr = (char *)string;
        return 0;
    }

    /* Use Amiga StrToLong for decimal base (base 10) */
    if (base == 10) {
        /* Reset strptr to start of number after sign */
        strptr = (char *)string;
        while (isspace(*strptr)) strptr++;
        if (*strptr == '+') strptr++;
        
        chars = StrToLong((STRPTR)strptr, &value);
        if (chars == -1) {
            /* No conversion performed */
            if (ptr != NULL) *ptr = (char *)string;
            return 0;
        }
        
        /* Convert signed to unsigned, handling negative values */
        if (value < 0) {
            /* For strtoul, negative values are treated as large positive values */
            retval = (unsigned long)value;
        } else {
            retval = (unsigned long)value;
        }
        
        /* Set endptr to point after converted characters */
        if (ptr != NULL) *ptr = (char *)(strptr + chars);
        return retval;
    }

    /* For non-decimal bases, use manual conversion */
    /* Convert digits */
    do {
        digit = toint(c);
        if (digit >= 0 && digit < base) {
            /* Check for overflow before multiplication */
            if (retval > (ULONG_MAX - digit) / base) {
                errno = ERANGE;
                retval = ULONG_MAX;
                break;
            }
            retval = (retval * base) + digit;
            i++;
        } else {
            break;
        }
    } while ((c = *(++strptr)) != 0);

    /* Set end pointer */
    if (ptr != NULL) {
        if (i > 0)
            *ptr = strptr;
        else
            *ptr = (char *)string;
    }

    return retval;
}

#else
/* Empty - function provided by SAS/C sc.lib */
#endif
