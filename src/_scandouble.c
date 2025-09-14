/*
 * SPDX-License-Identifier: BSD-2-Clause
 * _scandouble.c - Enhanced floating point scanning functions
 *
 * Based on PDC mscanf.c by J.A. Lydiatt
 * Enhanced for unix.lib by amigazen project
 * Copyright (C) 2025 by amigazen project
 */

#ifndef _WITH_SCLIB

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include "amiga.h"

/* Internal constants */
#define TRUE    1
#define FALSE   0

/**
 * @brief Scan floating point number from input
 * @param get Function to get next character
 * @param unget Function to unget character
 * @param maxlen Maximum length to scan
 * @return Pointer to scanned double value
 * 
 * Enhanced version of _scandouble function with better error handling.
 */
double *_scandouble(int (*get)(void), int (*unget)(int), int maxlen, double *result)
{
    double value = 0.0;
    double frac;
    int digit, exp;
    int negative, ch, i, nexp;

    *result = value;
    exp = 0;

    /* Skip leading whitespace */
    do {
        ch = (*get)();
        if (ch == EOF) {
            return NULL;
        }
    } while (isspace(ch));

    /* Check for sign */
    if (!(negative = (ch == '-'))) {
        if (ch != '+') {
            (*unget)(ch);
        }
    }

    /* Parse integer part */
    for (i = 0; i < maxlen; i++) {
        ch = (*get)();
        if (ch == EOF) {
            goto done;
        }
        digit = ch - '0';
        if ((digit >= 0) && (digit < 10)) {
            value = value * 10 + digit;
        } else {
            break;
        }
    }

    /* Parse fractional part */
    if (ch == '.') {
        frac = 1.0;
        for (; i < maxlen; i++) {
            ch = (*get)();
            if (ch == EOF) {
                goto done;
            }
            digit = ch - '0';
            if (digit < 0 || digit > 9) {
                break;
            } else {
                frac /= 10.0;
                value += frac * digit;
            }
        }
    }

    /* Parse exponent */
    if (ch == 'E' || ch == 'e') {
        ch = (*get)();
        nexp = 0;
        if (ch == '-' || ch == '+') {
            nexp = (ch == '-');
        } else {
            (*unget)(ch);
        }
        exp = 0;
        for (; i < maxlen; i++) {
            ch = (*get)();
            if (ch == EOF) {
                goto done;
            }
            digit = ch - '0';
            if (digit < 0 || digit > 9) {
                break;
            } else {
                exp = exp * 10 + digit;
            }
        }
        if (nexp) {
            while (exp > 0) {
                value /= 10.0;
                --exp;
            }
        } else {
            while (exp > 0) {
                value *= 10.0;
                --exp;
            }
        }
    }
    (*unget)(ch);

done:
    if (negative) {
        *result = -value;
    } else {
        *result = value;
    }
    return result;
}

#else
/* Empty - functions provided by SAS/C sc.lib */
#endif
