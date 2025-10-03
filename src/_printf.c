/*
 * Enhanced printf implementation
 * 
 * SPDX-License-Identifier: BSD-2-Clause
 * Copyright (c) 2025 amigazen project
 */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

#include "amiga.h"
#include "stdio.h"

/* Forward declarations for floating point functions */
extern char *_ftoa(double num, int precision);
extern char *_etoa(double num, int precision);
extern char *_gtoa(double num, int precision);
extern char *_justify(char *str, int width, int pad);

/* Number string for conversions */
static char _numstr[] = "0123456789ABCDEF";

/* Forward declarations */
static char *_strlwr(char *string);
static char *_ultoa(unsigned long n, char *buffer, int radix);
static char *_ltoa(long n, char *buffer, int radix);
static char *strrv(char *str);
static int _prtfld(char *op, int (*put)(int, void *), unsigned char *buf, 
                   int ljustf, char sign, char pad, int width, int preci);

/*
 * _strlwr - Convert string to lowercase
 */
static char *_strlwr(char *string)
{
    char *p = string;
    
    while (*string) {
        *string = tolower(*string);
        string++;
    }
    return p;
}

/*
 * strrv - Reverse string in place
 */
static char *strrv(char *str)
{
    char *start = str;
    char *end = str + strlen(str) - 1;
    char temp;
    
    while (start < end) {
        temp = *start;
        *start = *end;
        *end = temp;
        start++;
        end--;
    }
    return str;
}

/*
 * _ultoa - Convert unsigned long to string in given radix
 */
static char *_ultoa(unsigned long n, char *buffer, int radix)
{
    char *p = buffer;
    
    do {
        *p++ = _numstr[n % radix];
    } while ((n /= radix) > 0);
    
    *p = '\0';
    return strrv(buffer);
}

/*
 * _ltoa - Convert long to string in given radix
 */
static char *_ltoa(long n, char *buffer, int radix)
{
    char *p = buffer;
    
    if (n < 0) {
        *p++ = '-';
        n = -n;
    }
    _ultoa(n, p, radix);
    return buffer;
}

/*
 * _prtfld - Output formatted field with padding and justification
 */
static int _prtfld(char *op, int (*put)(int, void *), unsigned char *buf, 
                   int ljustf, char sign, char pad, int width, int preci)
{
    int cnt = 0, len;
    unsigned char ch;
    
    len = strlen(buf);
    
    if (*buf == '-') {
        sign = *buf++;
    } else if (sign) {
        len++;
    }
    
    if ((preci != -1) && (len > preci)) {
        len = preci;
    }
    
    if (width < len) {
        width = len;
    }
    
    width -= len;
    
    while (width || len) {
        if (!ljustf && width) {
            if (len && sign && (pad == '0')) {
                goto showsign;
            }
            ch = pad;
            --width;
        } else if (len) {
            if (sign) {
showsign:
                (*put)(sign, op);
                sign = 0;
            }
            ch = *buf++;
            --len;
        } else {
            ch = ' ';
            --width;
        }
        (*put)(ch, op);
    }
    
    return cnt;
}

/*
 * _printf - Core printf implementation
 * Supports: %d, %u, %x, %X, %c, %s, %b (BSTR), %f, %e, %g, %o, %p
 */
int _printf(void *op, int (*put)(int, void *), const char *fmt, va_list args)
{
    char *p;
    char *sval;
    long lval;
    unsigned long ulval;
    double dval;
    char cval;
    char numbuf[32];
    int width, preci, ljustf, sign, pad;
    int cnt = 0;
    
    for (p = (char *)fmt; *p; p++) {
        if (*p != '%') {
            (*put)(*p, op);
            cnt++;
            continue;
        }
        
        /* Process format specifier */
        p++;
        
        /* Parse flags */
        ljustf = FALSE;
        sign = 0;
        pad = ' ';
        
        while (*p == '-' || *p == '+' || *p == ' ' || *p == '#' || *p == '0') {
            if (*p == '-') ljustf = TRUE;
            if (*p == '+') sign = '+';
            if (*p == ' ') sign = ' ';
            if (*p == '0') pad = '0';
            p++;
        }
        
        /* Parse width */
        width = -1;
        if (isdigit(*p)) {
            width = 0;
            while (isdigit(*p)) {
                width = width * 10 + (*p - '0');
                p++;
            }
        }
        
        /* Parse precision */
        preci = -1;
        if (*p == '.') {
            p++;
            preci = 0;
            while (isdigit(*p)) {
                preci = preci * 10 + (*p - '0');
                p++;
            }
        }
        
        /* Parse length modifiers */
        while (*p == 'h' || *p == 'l' || *p == 'L' || *p == 'z' || *p == 't') {
            p++;
        }
        
        /* Process type specifier */
        switch (*p) {
            case 'd':
            case 'i':
                lval = va_arg(args, long);
                _ltoa(lval, numbuf, 10);
                cnt += _prtfld(op, put, numbuf, ljustf, sign, pad, width, preci);
                break;
                
            case 'u':
                ulval = va_arg(args, unsigned long);
                _ultoa(ulval, numbuf, 10);
                cnt += _prtfld(op, put, numbuf, ljustf, 0, pad, width, preci);
                break;
                
            case 'x':
            case 'X':
                ulval = va_arg(args, unsigned long);
                _ultoa(ulval, numbuf, 16);
                if (*p == 'X') {
                    _strlwr(numbuf);
                }
                cnt += _prtfld(op, put, numbuf, ljustf, 0, pad, width, preci);
                break;
                
            case 'o':
                ulval = va_arg(args, unsigned long);
                _ultoa(ulval, numbuf, 8);
                cnt += _prtfld(op, put, numbuf, ljustf, 0, pad, width, preci);
                break;
                
            case 'c':
                cval = va_arg(args, int);
                (*put)(cval, op);
                cnt++;
                break;
                
            case 's':
                sval = va_arg(args, char *);
                if (sval == NULL) {
                    sval = "(null)";
                }
                if (preci != -1) {
                    sval[preci] = '\0';
                }
                cnt += _prtfld(op, put, sval, ljustf, 0, pad, width, -1);
                break;
                
            case 'b':
                /* BSTR support - AmigaOS specific */
                sval = va_arg(args, char *);
                if (sval) {
                    int len = *((unsigned char *)sval);
                    sval++;
                    if (preci != -1 && len > preci) {
                        len = preci;
                    }
                    sval[len] = '\0';
                    cnt += _prtfld(op, put, sval, ljustf, 0, pad, width, -1);
                }
                break;
                
            case 'f':
            case 'F':
                /* Floating point - use _ftoa for better formatting */
                dval = va_arg(args, double);
                if (isnan(dval)) {
                    strcpy(numbuf, "nan");
                } else if (isinf(dval)) {
                    strcpy(numbuf, dval < 0 ? "-inf" : "inf");
                } else {
                    /* Use _ftoa for better floating point formatting */
                    char *float_str = _ftoa(dval, (preci >= 0) ? preci : 6);
                    strcpy(numbuf, float_str);
                }
                cnt += _prtfld(op, put, numbuf, ljustf, 0, pad, width, preci);
                break;
                
            case 'e':
            case 'E':
                /* Scientific notation - use _etoa for better formatting */
                dval = va_arg(args, double);
                if (isnan(dval)) {
                    strcpy(numbuf, "nan");
                } else if (isinf(dval)) {
                    strcpy(numbuf, dval < 0 ? "-inf" : "inf");
                } else {
                    /* Use _etoa for better scientific notation formatting */
                    char *float_str = _etoa(dval, (preci >= 0) ? preci : 6);
                    strcpy(numbuf, float_str);
                }
                cnt += _prtfld(op, put, numbuf, ljustf, 0, pad, width, preci);
                break;
                
            case 'g':
            case 'G':
                /* General format - use _gtoa for better formatting */
                dval = va_arg(args, double);
                if (isnan(dval)) {
                    strcpy(numbuf, "nan");
                } else if (isinf(dval)) {
                    strcpy(numbuf, dval < 0 ? "-inf" : "inf");
                } else {
                    /* Use _gtoa for better general format formatting */
                    char *float_str = _gtoa(dval, (preci >= 0) ? preci : 6);
                    strcpy(numbuf, float_str);
                }
                cnt += _prtfld(op, put, numbuf, ljustf, 0, pad, width, preci);
                break;
                
            case 'p':
                /* Pointer support */
                ulval = (unsigned long)va_arg(args, void *);
                strcpy(numbuf, "0x");
                _ultoa(ulval, numbuf + 2, 16);
                cnt += _prtfld(op, put, numbuf, ljustf, 0, pad, width, -1);
                break;
                
            case '%':
                (*put)('%', op);
                cnt++;
                break;
                
            default:
                /* Unknown format - output as-is */
                (*put)('%', op);
                (*put)(*p, op);
                cnt += 2;
                break;
        }
    }
    
    return cnt;
}
