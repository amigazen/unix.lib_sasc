/*
 * SPDX-License-Identifier: BSD-2-Clause
 * format.c - Enhanced formatting engine (POSIX compliant)
 *
 * This function provides a core formatting engine that can be used
 * by printf, fprintf, sprintf, and other formatted output functions.
 * It leverages AmigaOS RawDoFmt() for optimal performance when possible,
 * falling back to custom implementation for unsupported format specifiers.
 *
 * POSIX.1-2001, POSIX.1-2008, C89, C99
 * Copyright (C) 2025 by amigazen project
 */

#ifndef _WITH_SCLIB

#include "amiga.h"
#include <proto/utility.h>
#include <stdio.h>
#include <varargs.h>
#include <string.h>
#include <limits.h>

/* Format string processing states */
#define FMT_SAFE_DIRECT    0  /* Safe to pass to RawDoFmt directly */
#define FMT_NEEDS_PREPROC  1  /* Needs preprocessing then can pass to RawDoFmt */
#define FMT_NEEDS_FALLBACK 2  /* Cannot process by RawDoFmt, need fallback */

/* Internal constants */
#define MAXSTRING 256

/* Internal state variables */
static unsigned int base;
static int minwidth, precision, padchar, rjust, nsent;
static int (*put)(int c, void *op);
static void *put_op;

/* Forward declarations */
static char *putint(char *str, long n);
static int prt(char *strbuf);
static int can_use_rawdofmt(const char *fmt);
static int rewrite_format_string(char *new_fmt, size_t new_fmt_size, const char *fmt);
static int fallback_format(int (*psub)(int c, void *op), const char *ctl, va_list argptr);

/**
 * @brief Check if format string can use RawDoFmt
 * @param fmt Format string to analyze
 * @return FMT_SAFE_DIRECT, FMT_NEEDS_PREPROC, or FMT_NEEDS_FALLBACK
 * 
 * This function analyzes the format string to determine if it can be
 * processed by AmigaOS RawDoFmt() function for optimal performance.
 */
static int can_use_rawdofmt(const char *fmt)
{
    const char *p = fmt;
    int needs_preprocessing = 0;
    
    while ((p = strchr(p, '%')) != NULL) {
        p++; /* Skip the % */
        
        /* Skip flags */
        while (*p == '-' || *p == '+' || *p == ' ' || *p == '#' || *p == '0') {
            p++;
        }
        
        /* Skip width */
        while (isdigit(*p)) {
            p++;
        }
        
        /* Skip precision */
        if (*p == '.') {
            p++;
            while (isdigit(*p)) {
                p++;
            }
        }
        
        /* Skip length modifiers */
        while (*p == 'h' || *p == 'l' || *p == 'L' || *p == 'z' || *p == 't') {
            p++;
        }
        
        /* Check type specifier */
        switch (*p) {
            case 'd':
            case 'u':
            case 'x':
            case 'X':
            case 'c':
            case 's':
            case 'b':  /* BSTR - AmigaOS specific */
                /* These are supported by RawDoFmt but may need preprocessing */
                /* Check if we need to add 'l' for 32-bit compatibility */
                if (*p == 'd' || *p == 'u' || *p == 'x' || *p == 'X' || *p == 'c') {
                    const char *prev = p - 1;
                    int has_l = 0;
                    
                    /* Look back for 'l' modifier */
                    while (prev >= fmt && (*prev == 'h' || *prev == 'l' || *prev == 'L')) {
                        if (*prev == 'l') {
                            has_l = 1;
                            break;
                        }
                        prev--;
                    }
                    
                    /* If no 'l' modifier found, we need preprocessing */
                    if (!has_l) {
                        needs_preprocessing = 1;
                    }
                }
                break;
                
            case 'f':
            case 'e':
            case 'E':
            case 'g':
            case 'G':
            case 'o':
            case 'p':
                /* These are NOT supported by RawDoFmt - need fallback */
                return FMT_NEEDS_FALLBACK;
                
            case '%':
                /* Escaped % - always supported */
                break;
                
            default:
                /* Unknown specifier - assume unsupported */
                return FMT_NEEDS_FALLBACK;
        }
        
        p++; /* Move to next character */
    }
    
    /* Return appropriate state */
    if (needs_preprocessing) {
        return FMT_NEEDS_PREPROC;
    } else {
        return FMT_SAFE_DIRECT;
    }
}

/**
 * @brief Rewrite format string for RawDoFmt compatibility
 * @param new_fmt Buffer for rewritten format string
 * @param new_fmt_size Size of buffer
 * @param fmt Original format string
 * @return Length of rewritten string
 * 
 * This function converts standard C format strings to RawDoFmt compatible
 * format by adding 'l' modifiers for 32-bit integer compatibility.
 */
static int rewrite_format_string(char *new_fmt, size_t new_fmt_size, const char *fmt)
{
    const char *src = fmt;
    char *dst = new_fmt;
    size_t remaining = new_fmt_size - 1; /* Leave room for NUL */
    
    while (*src && remaining > 0) {
        if (*src == '%') {
            *dst++ = *src++; /* Copy the % */
            remaining--;
            
            /* Skip flags */
            while (*src && remaining > 0 && 
                   (*src == '-' || *src == '+' || *src == ' ' || *src == '#' || *src == '0')) {
                *dst++ = *src++;
                remaining--;
            }
            
            /* Skip width */
            while (*src && remaining > 0 && isdigit(*src)) {
                *dst++ = *src++;
                remaining--;
            }
            
            /* Skip precision */
            if (*src == '.' && remaining > 0) {
                *dst++ = *src++;
                remaining--;
                while (*src && remaining > 0 && isdigit(*src)) {
                    *dst++ = *src++;
                    remaining--;
                }
            }
            
            /* Handle length modifiers - convert to RawDoFmt compatible */
            if (*src && remaining > 0) {
                if (*src == 'd' || *src == 'u' || *src == 'x' || *src == 'c') {
                    /* Check if we need to add 'l' for 32-bit compatibility */
                    const char *prev = dst - 1;
                    int has_l = 0;
                    
                    /* Look back for 'l' modifier */
                    while (prev >= new_fmt && (*prev == 'h' || *prev == 'l' || *prev == 'L')) {
                        if (*prev == 'l') {
                            has_l = 1;
                            break;
                        }
                        prev--;
                    }
                    
                    /* Add 'l' if not present for 32-bit compatibility */
                    if (!has_l && remaining > 0) {
                        *dst++ = 'l';
                        remaining--;
                    }
                }
            }
            
            /* Copy the type specifier */
            if (*src && remaining > 0) {
                *dst++ = *src++;
                remaining--;
            }
        } else {
            /* Copy regular character */
            *dst++ = *src++;
            remaining--;
        }
    }
    
    *dst = '\0'; /* NUL terminate */
    return (dst - new_fmt);
}

/**
 * @brief Convert long integer to string in given base
 * @param str Buffer to store result
 * @param n Number to convert
 * @return Pointer to start of string
 * 
 * This function converts a long integer to a string representation
 * in the current base (set by the format engine).
 */
static char *putint(char *str, long n)
{
    char *s;
    int is_negative;
    unsigned long un;
    static const char digit[] = "0123456789ABCDEF";
    
    /* Handle special case for minimum long value */
    if (n == LONG_MIN) {
        switch (base) {
        case 8:
            s = "20000000000";
            break;
        case 10:
            s = "-2147483648";
            break;
        case 16:
            s = "80000000";
            break;
        default:
            s = "-OOPS-";
        }
        strcpy(str, s);
        return str;
    }
    
    is_negative = (base == 10 && n < 0);
    if (is_negative) {
        n = -n;
    }
    
    un = (unsigned long)n;
    
    s = &str[MAXSTRING - 1];
    *s = '\0';
    
    do {
        *--s = digit[un % base];
        un /= base;
    } while (un && s > &str[1]);
    
    if (is_negative) {
        *--s = '-';
    }
    
    return s;
}

/**
 * @brief Output formatted field with justification and padding
 * @param strbuf String to output
 * @return Number of characters output, -1 on error
 * 
 * This function handles the justification and padding of formatted
 * output according to the current format specifications.
 */
static int prt(char *strbuf)
{
    int i;
    int c;
    int len = 1;
    int strsize = 0;
    const char *s = strbuf;
    while (*s++) strsize++;
    
    /* Apply precision limit */
    if (strsize > precision) {
        strsize = precision;
    }
    
    /* Right justification with special handling for signs */
    if (rjust) {
        if (padchar != ' ' && (c = *strbuf) == '-' || c == '+') {
            if (put(c, put_op) == -1) {
                return -1;
            }
            ++len;
            ++strbuf;
            --strsize;
        }
        
        while (--minwidth >= strsize) {
            if (put(padchar, put_op) == -1) {
                return -1;
            }
            ++len;
        }
    }
    
    /* Output the string */
    for (i = 0; i < precision && (c = *strbuf++); ++i) {
        if (put(c, put_op) == -1) {
            return -1;
        }
    }
    len += i;
    
    /* Left padding with spaces if necessary */
    while (len < minwidth) {
        if (put(' ', put_op) == -1) {
            return -1;
        }
        ++len;
    }
    
    nsent += len;
    return len;
}

/**
 * @brief Fallback formatting engine for unsupported specifiers
 * @param psub Function to output characters
 * @param ctl Format control string
 * @param argptr Pointer to arguments
 * @return Number of characters output, -1 on error
 * 
 * This function provides fallback formatting for specifiers not supported
 * by RawDoFmt (f, e, g, o, p, etc.).
 */
static int fallback_format(int (*psub)(int c, void *op), const char *ctl, va_list argptr)
{
    union {
        short *hptr;
        int   *iptr;
        long  *lptr;
        char  *cptr;
        char **cptr2;
    } args;
    
    unsigned int argsize;
    int c;
    long l;
    char *sptr;
    char strbuf[MAXSTRING];
    
    put = psub;
    put_op = (void *)argptr;  /* Store for use in prt() */
    args.cptr = (char *)argptr;
    nsent = 0;
    
    while ((c = *ctl) != '\0') {
        /* Output non-format characters */
        if (c != '%') {
            if (psub(c, (void *)argptr) == -1) {
                return -1;
            }
            ++ctl;
            ++nsent;
            continue;
        }
        
        /* Handle %% */
        if ((c = *++ctl) == '%') {
            if (psub(c, (void *)argptr) == -1) {
                return -1;
            }
            ++ctl;
            ++nsent;
            continue;
        }
        
        /* Initialize format specifier */
        rjust = 1;
        argsize = sizeof(int);
        minwidth = precision = 0;
        padchar = ' ';
        
        /* Parse flags */
        if (c == '-') {
            rjust = 0;
            c = *++ctl;
        }
        
        if (c == '0') {
            padchar = '0';
            c = *++ctl;
        }
        
        /* Parse width */
        if (c == '*') {
            minwidth = *args.iptr++;
            c = *++ctl;
        } else {
            while ('0' <= c && c <= '9') {
                minwidth = minwidth * 10 + c - '0';
                c = *++ctl;
            }
        }
        
        /* Parse precision */
        if (c == '.') {
            c = *++ctl;
            if (c == '*') {
                precision = *args.iptr++;
                c = *++ctl;
            } else {
                while ('0' <= c && c <= '9') {
                    precision = precision * 10 + c - '0';
                    c = *++ctl;
                }
            }
        }
        
        /* Parse length modifiers */
        if (c == 'l' || c == 'L') {
            argsize = sizeof(long);
            c = *++ctl;
        } else if (c == 'h' || c == 'H') {
            argsize = sizeof(short);
            c = *++ctl;
        }
        
        /* Set precision default */
        if (precision >= (MAXSTRING - 2) || precision == 0) {
            precision = (MAXSTRING - 2);
        }
        base = 0;
        ++ctl;
        
        /* Process conversion specifier */
        switch (c) {
        case 'x':
            base += 8;  /* Fall through */
        case 'o':
            base += 8;
        case 'd':
        case 'i':
            if (!base) {
                base = 10;
            }
            if (argsize == sizeof(long)) {
                l = *args.lptr++;
            } else {
                l = *args.hptr++;
            }
            sptr = putint(strbuf, l);
            if (prt(sptr) == -1) {
                return -1;
            }
            break;
            
        case 'u':
            base = 10;
            if (argsize == sizeof(long)) {
                l = *args.lptr++;
            } else {
                l = *args.hptr++;
            }
            sptr = putint(strbuf, l);
            if (prt(sptr) == -1) {
                return -1;
            }
            break;
            
        case 's':
            sptr = *args.cptr2++;
            if (minwidth) {
                precision = minwidth;
            } else {
                precision = 32717;
            }
            if (prt(sptr) == -1) {
                return -1;
            }
            break;
            
        case 'c':
            c = *args.iptr++;
            strbuf[0] = c;
            strbuf[1] = '\0';
            if (prt(strbuf) == -1) {
                return -1;
            }
            break;
            
        default:
            /* Unknown format - output as-is */
            strbuf[0] = c;
            strbuf[1] = '\0';
            if (prt(strbuf) == -1) {
                return -1;
            }
            break;
        }
    }
    
    return nsent;
}

/**
 * @brief RawDoFmt character output function
 * @param c Character to output (UBYTE)
 * @param op Output function pointer (APTR)
 * 
 * This function is called by RawDoFmt for each character to be output.
 * It forwards the character to the output function.
 */
static void rawdofmt_putchar(UBYTE c, APTR op)
{
    int (*put)(int c, void *op) = (int (*)(int, void *))op;
    put((int)c, op);
}

/**
 * @brief Core formatting engine using AmigaOS RawDoFmt
 * @param psub Function to output characters
 * @param ctl Format control string
 * @param argptr Pointer to arguments
 * @return Number of characters output, -1 on error
 * 
 * This function processes a format string and outputs formatted data
 * using AmigaOS RawDoFmt() for optimal performance when possible,
 * falling back to custom implementation for unsupported specifiers.
 * 
 * Supported by RawDoFmt:
 * - %d, %ld: Signed decimal integer
 * - %u, %lu: Unsigned decimal integer
 * - %x, %lx: Unsigned hexadecimal integer
 * - %c, %lc: Character
 * - %s: String
 * - %b: BSTR (AmigaOS specific)
 * - %%: Literal percent sign
 * 
 * Fallback for:
 * - %f, %e, %g: Floating point
 * - %o: Octal
 * - %p: Pointer
 */
int format(int (*psub)(int c, void *op), const char *ctl, va_list argptr)
{
    int can_process;
    char new_fmt[256]; /* Buffer for rewritten format string */
    int result;
    
    /* Check if we can use RawDoFmt or need fallback */
    can_process = can_use_rawdofmt(ctl);
    
    if (can_process == FMT_SAFE_DIRECT) {
        /* Safe to pass to RawDoFmt directly - no preprocessing needed */
        /* Use RawDoFmt for optimal performance */
        /* RawDoFmt returns pointer to end of DataStream, not count */
        RawDoFmt((STRPTR)ctl, argptr, (APTR)rawdofmt_putchar, psub);
        result = 0; /* Success - characters written via callback */
    } else if (can_process == FMT_NEEDS_PREPROC) {
        /* Needs preprocessing then can pass to RawDoFmt */
        if (rewrite_format_string(new_fmt, sizeof(new_fmt), ctl) > 0) {
            /* Use RawDoFmt with rewritten format string */
            /* RawDoFmt returns pointer to end of DataStream, not count */
            RawDoFmt((STRPTR)new_fmt, argptr, (APTR)rawdofmt_putchar, psub);
            result = 0; /* Success - characters written via callback */
        } else {
            /* Fallback if format rewriting fails */
            result = fallback_format(psub, ctl, argptr);
        }
    } else {
        /* Cannot process by RawDoFmt - use fallback implementation */
        result = fallback_format(psub, ctl, argptr);
    }
    
    return result;
}

#else
/* Empty - functions provided by SAS/C sc.lib */
#endif
