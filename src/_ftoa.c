/*
 * SPDX-License-Identifier: BSD-2-Clause
 * _ftoa.c - floating point formatting functions
 *
 * Based on PDC mprintf.c by J.A. Lydiatt
 * Copyright (C) 2025 by amigazen project
 */

#ifndef _WITH_SCLIB

#include <stdio.h>
#include <math.h>
#include <string.h>
#include "amiga.h"

/* Internal constants */
#define TRUE    1
#define FALSE   0

/* Character classification macros */
#define isupper(x)  ((x) >= 'A' && (x) <= 'Z')
#define islower(x)  ((x) >= 'a' && (x) <= 'z')
#define toupper(x)  ((x) - 'a' + 'A')
#define tolower(x)  ((x) - 'A' + 'a')
#define isdigit(x)  ((x) >= '0' && (x) <= '9')

/* Internal buffer for floating point conversions */
static char _float_buffer[128];
static char _hexdigits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

/**
 * @brief Convert integer to string in given base
 * @param num Number to convert
 * @param base Base for conversion (8, 10, 16)
 * @return Pointer to converted string
 * 
 * Enhanced version of _itoa function with better error handling.
 */
char *_itoa(int num, unsigned int base)
{
    char *ptr;
    int negative, digit;
    unsigned int un;

    ptr = &_float_buffer[sizeof(_float_buffer) - 1];
    *ptr = '\0';

    /* Handle special case for minimum integer value */
    if (num == 0x80000000) {
        switch (base) {
        case 8:
            return "20000000000";
        case 10:
            return "-2147483648";
        case 16:
            return "80000000";
        default:
            return "-OOPS-";
        }
    }

    negative = (num < 0);
    if (negative) {
        un = (unsigned int) -num;
    } else {
        un = (unsigned int) num;
    }

    do {
        *(--ptr) = _hexdigits[un % base];
        un /= base;
    } while (un && ptr > &_float_buffer[1]);

    if (negative) {
        *(--ptr) = '-';
    }

    return ptr;
}

/**
 * @brief Convert floating point to string with fixed precision
 * @param num Number to convert
 * @param precision Number of decimal places
 * @return Pointer to converted string
 * 
 * Enhanced version of _ftoa function with better rounding.
 */
char *_ftoa(double num, int precision)
{
    char *ptr;
    int i, negative, digit;
    double frac, intp, temp;

    if (precision > 32) {
        precision = 6;
    }

    negative = (num < 0.0);
    if (negative) {
        num = -num;
    }

    /* Add rounding factor */
    num += 0.5 / pow(10.0, (double)precision);
    intp = floor(num);
    frac = num - intp;

    ptr = _float_buffer;
    for (i = 0; i < precision; i++) {
        frac *= 10.0;
        digit = floor(frac);
        *ptr++ = (digit + '0');
        frac -= digit;
    }
    *ptr = '\0';

    strcpy(&_float_buffer[63 - precision], _float_buffer);
    ptr = &_float_buffer[63 - precision];

    *(--ptr) = '.';

    do {
        temp = floor(intp / 10.0);
        digit = intp - (temp * 10.0);
        *(--ptr) = (digit + '0');
        intp = temp;
    } while (intp > 0.0);

    if (negative) {
        *(--ptr) = '-';
    }

    return ptr;
}

/**
 * @brief Convert floating point to scientific notation string
 * @param num Number to convert
 * @param precision Number of decimal places
 * @return Pointer to converted string
 * 
 * Enhanced version of _etoa function with better exponent handling.
 */
char *_etoa(double num, int precision)
{
    char *ptr;
    int i, negative, digit, exp;
    double frac, intp, temp;

    if (precision > 32) {
        precision = 6;
    }

    negative = (num < 0.0);
    if (negative) {
        num = -num;
    }

    exp = 0;

    if (num != 0.0) {
        while (num >= 10.0) {
            num /= 10.0;
            exp++;
        }

        while (num < 1.0) {
            num *= 10.0;
            exp--;
        }
    }

    /* Add rounding factor */
    num += 0.5 / pow(10.0, (double)precision);
    intp = floor(num);
    frac = num - intp;

    ptr = _float_buffer;
    for (i = 0; i < precision; i++) {
        frac *= 10.0;
        digit = floor(frac);
        *ptr++ = (digit + '0');
        frac -= digit;
    }

    *ptr++ = 'E';
    if (exp >= 0) {
        *ptr++ = '+';
    } else {
        *ptr++ = '-';
        exp = -exp;
    }
    for (i = 2; i >= 0; i--) {
        digit = exp % 10;
        ptr[i] = (digit + '0');
        exp /= 10;
    }
    ptr[3] = '\0';

    strcpy(&_float_buffer[63 - (precision + 6)], _float_buffer);
    ptr = &_float_buffer[63 - (precision + 6)];
    *(--ptr) = '.';

    do {
        temp = floor(intp / 10.0);
        digit = intp - (temp * 10.0);
        *(--ptr) = (digit + '0');
        intp = temp;
    } while (intp > 0.0);

    if (negative) {
        *(--ptr) = '-';
    }

    return ptr;
}

/**
 * @brief Convert floating point to general format string
 * @param num Number to convert
 * @param precision Number of significant digits
 * @return Pointer to converted string
 * 
 * Enhanced version of _gtoa function with better format selection.
 */
char *_gtoa(double num, int precision)
{
    char *ptr;
    double fsiz;

    fsiz = (num < 0.0 ? -num : num);

    /* Use scientific notation for very large or very small numbers */
    if (fsiz > 1.0E+20 || fsiz < 1.0E-20) {
        return _etoa(num, precision);
    }

    ptr = _ftoa(num, precision);
    strcpy(_float_buffer, ptr);

    /* Remove trailing zeros */
    if (precision > 0) {
        ptr = _float_buffer;
        while (*ptr) {
            ptr++;
        }
        --ptr;
        while (*ptr == '0') {
            *ptr-- = '\0';
        }
    }
    return _float_buffer;
}

/**
 * @brief Left justify string with padding
 * @param str String to justify
 * @param width Target width
 * @param pad Padding character
 * @return Pointer to justified string
 * 
 * Enhanced version of _leftjust function.
 */
char *_leftjust(char *str, int width, int pad)
{
    char *ptr;
    int len;

    len = 0;
    ptr = _float_buffer;
    for (; *str; len++) {
        *ptr++ = *str++;
    }
    while (len < width) {
        *ptr++ = pad;
        len++;
    }
    *ptr = '\0';
    return _float_buffer;
}

/**
 * @brief Right justify string with padding
 * @param str String to justify
 * @param width Target width
 * @param pad Padding character
 * @return Pointer to justified string
 * 
 * Enhanced version of _rightjust function.
 */
char *_rightjust(char *str, int width, int pad)
{
    char *ptr;
    int len;

    len = strlen(str);
    ptr = _float_buffer;
    while (len++ < width) {
        *ptr++ = pad;
    }
    strcpy(ptr, str);

    return _float_buffer;
}

/**
 * @brief Justify string based on width sign
 * @param str String to justify
 * @param width Target width (positive = right, negative = left)
 * @param pad Padding character
 * @return Pointer to justified string
 * 
 * Enhanced version of _justify function.
 */
char *_justify(char *str, int width, int pad)
{
    if (width > 0) {
        return _rightjust(str, width, pad);
    } else if (width < 0) {
        return _leftjust(str, -width, pad);
    }
    return str;
}

#else
/* Empty - functions provided by SAS/C sc.lib */
#endif
