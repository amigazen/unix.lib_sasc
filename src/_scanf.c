/*
 * Enhanced scanf implementation
 * 
 * SPDX-License-Identifier: BSD-2-Clause
 * Copyright (c) 2025 amigazen project
 */

#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>

#include "amiga.h"
#include "stdio.h"

/* Forward declarations for floating point scanning functions */
extern double *_scandouble(int (*get)(void), int (*unget)(int), int maxlen, double *result);

/* Internal constants - TRUE/FALSE provided by exec/types.h */

/* Number string for conversions */
static char _numstr[] = "0123456789ABCDEF";

/* Forward declarations */
static double fp_scan(int (*get)(void), int (*unget)(int), int *width);
static int skip_whitespace(int (*get)(void), int (*unget)(int));

/*
 * skip_whitespace - Skip whitespace characters
 */
static int skip_whitespace(int (*get)(void), int (*unget)(int))
{
    int c;
    int skipped = 0;
    
    while ((c = (*get)()) > 0 && isspace(c)) {
        skipped++;
    }
    
    if (c > 0) {
        (*unget)(c);
    }
    
    return skipped;
}

/*
 * fp_scan - Scan floating point number
 */
static double fp_scan(int (*get)(void), int (*unget)(int), int *width)
{
    double result = 0.0;
    double fraction = 0.0;
    double divisor = 1.0;
    int exponent = 0;
    int exp_sign = 1;
    int c;
    int has_digits = FALSE;
    int has_fraction = FALSE;
    int has_exponent = FALSE;
    
    /* Skip leading whitespace */
    skip_whitespace(get, unget);
    
    /* Check for sign */
    c = (*get)();
    if (c == '+' || c == '-') {
        if (c == '-') {
            exp_sign = -1;
        }
        c = (*get)();
    } else {
        (*unget)(c);
    }
    
    /* Parse integer part */
    while ((c = (*get)()) > 0 && isdigit(c)) {
        result = result * 10.0 + (c - '0');
        has_digits = TRUE;
        if (width) (*width)--;
    }
    
    if (c > 0) {
        (*unget)(c);
    }
    
    /* Parse fraction part */
    c = (*get)();
    if (c == '.') {
        has_fraction = TRUE;
        if (width) (*width)--;
        
        while ((c = (*get)()) > 0 && isdigit(c)) {
            fraction = fraction * 10.0 + (c - '0');
            divisor *= 10.0;
            has_digits = TRUE;
            if (width) (*width)--;
        }
        
        if (c > 0) {
            (*unget)(c);
        }
    } else if (c > 0) {
        (*unget)(c);
    }
    
    /* Parse exponent */
    c = (*get)();
    if (c == 'e' || c == 'E') {
        has_exponent = TRUE;
        if (width) (*width)--;
        
        c = (*get)();
        if (c == '+' || c == '-') {
            if (c == '-') {
                exp_sign = -1;
            }
            if (width) (*width)--;
            c = (*get)();
        }
        
        while (c > 0 && isdigit(c)) {
            exponent = exponent * 10 + (c - '0');
            if (width) (*width)--;
            c = (*get)();
        }
        
        if (c > 0) {
            (*unget)(c);
        }
    } else if (c > 0) {
        (*unget)(c);
    }
    
    /* Calculate final result */
    if (has_digits) {
        result += fraction / divisor;
        if (has_exponent) {
            while (exponent-- > 0) {
                if (exp_sign > 0) {
                    result *= 10.0;
                } else {
                    result /= 10.0;
                }
            }
        }
    }
    
    return result;
}

/*
 * _scanf - Core scanf implementation
 * Supports: %d, %u, %x, %X, %c, %s, %f, %e, %g, %o, %p, %n
 */
int _scanf(void *ip, int (*get)(), int (*unget)(), const char *fmt, va_list args)
{
    char *p;
    char *sval;
    int *ival;
    long *lval;
    unsigned long *ulval;
    double *dval;
    char *cval;
    int *nval;
    int width, store, neg, endnull;
    int cnt = 0;
    int c;
    
    if (!*fmt) {
        return 0;
    }
    
    c = (*get)(ip);
    while (c > 0) {
        store = FALSE;
        
        if (*fmt == '%') {
            /* Process format specifier */
            fmt++;
            
            /* Parse flags */
            width = -1;
            store = TRUE;
            endnull = TRUE;
            
            /* Parse width */
            if (isdigit(*fmt)) {
                width = 0;
                while (isdigit(*fmt)) {
                    width = width * 10 + (*fmt - '0');
                    fmt++;
                }
            }
            
            /* Parse length modifiers */
            while (*fmt == 'h' || *fmt == 'l' || *fmt == 'L' || *fmt == 'z' || *fmt == 't') {
                fmt++;
            }
            
            /* Process type specifier */
            switch (*fmt) {
                case 'd':
                case 'i':
                    /* Signed decimal integer */
                    lval = va_arg(args, long *);
                    if (lval && store) {
                        *lval = 0;
                        neg = FALSE;
                        
                        /* Skip whitespace */
                        skip_whitespace(get, unget);
                        
                        /* Check for sign */
                        c = (*get)(ip);
                        if (c == '+' || c == '-') {
                            if (c == '-') neg = TRUE;
                            c = (*get)(ip);
                        }
                        
                        /* Parse digits */
                        while (c > 0 && isdigit(c)) {
                            *lval = *lval * 10 + (c - '0');
                            c = (*get)(ip);
                        }
                        
                        if (c > 0) (*unget)(c);
                        if (neg) *lval = -*lval;
                        cnt++;
                    }
                    break;
                    
                case 'u':
                    /* Unsigned decimal integer */
                    ulval = va_arg(args, unsigned long *);
                    if (ulval && store) {
                        *ulval = 0;
                        
                        /* Skip whitespace */
                        skip_whitespace(get, unget);
                        
                        /* Parse digits */
                        c = (*get)(ip);
                        while (c > 0 && isdigit(c)) {
                            *ulval = *ulval * 10 + (c - '0');
                            c = (*get)(ip);
                        }
                        
                        if (c > 0) (*unget)(c);
                        cnt++;
                    }
                    break;
                    
                case 'x':
                case 'X':
                    /* Hexadecimal integer */
                    ulval = va_arg(args, unsigned long *);
                    if (ulval && store) {
                        *ulval = 0;
                        
                        /* Skip whitespace */
                        skip_whitespace(get, unget);
                        
                        /* Parse hex digits */
                        c = (*get)(ip);
                        while (c > 0 && isxdigit(c)) {
                            if (isdigit(c)) {
                                *ulval = *ulval * 16 + (c - '0');
                            } else {
                                *ulval = *ulval * 16 + (tolower(c) - 'a' + 10);
                            }
                            c = (*get)(ip);
                        }
                        
                        if (c > 0) (*unget)(c);
                        cnt++;
                    }
                    break;
                    
                case 'o':
                    /* Octal integer */
                    ulval = va_arg(args, unsigned long *);
                    if (ulval && store) {
                        *ulval = 0;
                        
                        /* Skip whitespace */
                        skip_whitespace(get, unget);
                        
                        /* Parse octal digits */
                        c = (*get)(ip);
                        while (c > 0 && c >= '0' && c <= '7') {
                            *ulval = *ulval * 8 + (c - '0');
                            c = (*get)(ip);
                        }
                        
                        if (c > 0) (*unget)(c);
                        cnt++;
                    }
                    break;
                    
                case 'c':
                    /* Character */
                    cval = va_arg(args, char *);
                    if (cval && store) {
                        c = (*get)(ip);
                        if (c > 0) {
                            *cval = c;
                            cnt++;
                        }
                    }
                    break;
                    
                case 's':
                    /* String */
                    sval = va_arg(args, char *);
                    if (sval && store) {
                        /* Skip whitespace */
                        skip_whitespace(get, unget);
                        
                        /* Parse string */
                        c = (*get)(ip);
                        while (c > 0 && !isspace(c)) {
                            *sval++ = c;
                            c = (*get)(ip);
                        }
                        
                        if (c > 0) (*unget)(c);
                        *sval = '\0';
                        cnt++;
                    }
                    break;
                    
                case 'f':
                case 'e':
                case 'g':
                case 'E':
                case 'F':
                case 'G':
                    /* Floating point - use _scandouble for better scanning */
                    dval = va_arg(args, double *);
                    if (dval && store) {
                        double result;
                        if (_scandouble(get, unget, (width > 0) ? width : 1024, &result) != NULL) {
                            *dval = result;
                            cnt++;
                        }
                    }
                    break;
                    
                case 'p':
                    /* Pointer */
                    ulval = va_arg(args, unsigned long *);
                    if (ulval && store) {
                        /* Skip whitespace */
                        skip_whitespace(get, unget);
                        
                        /* Check for 0x prefix */
                        c = (*get)(ip);
                        if (c == '0') {
                            c = (*get)(ip);
                            if (c == 'x' || c == 'X') {
                                c = (*get)(ip);
                            } else {
                                (*unget)(c);
                                c = '0';
                            }
                        }
                        
                        /* Parse hex digits */
                        *ulval = 0;
                        while (c > 0 && isxdigit(c)) {
                            if (isdigit(c)) {
                                *ulval = *ulval * 16 + (c - '0');
                            } else {
                                *ulval = *ulval * 16 + (tolower(c) - 'a' + 10);
                            }
                            c = (*get)(ip);
                        }
                        
                        if (c > 0) (*unget)(c);
                        cnt++;
                    }
                    break;
                    
                case 'n':
                    /* Number of characters read so far */
                    nval = va_arg(args, int *);
                    if (nval && store) {
                        *nval = cnt;
                    }
                    break;
                    
                case '%':
                    /* Literal % */
                    c = (*get)(ip);
                    if (c == '%') {
                        cnt++;
                    } else if (c > 0) {
                        (*unget)(c);
                    }
                    break;
                    
                default:
                    /* Unknown format - skip */
                    break;
            }
            
            fmt++;
        } else if (*fmt == c) {
            /* Literal character match */
            fmt++;
            c = (*get)(ip);
        } else {
            /* No match */
            break;
        }
    }
    
    return cnt;
}
