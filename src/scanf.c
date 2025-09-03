/*
 * Enhanced scanf implementation
 * 
 * SPDX-License-Identifier: BSD-2-Clause
 * Copyright (c) 2025 amigazen project
 */

#include <stdio.h>
#include <stdarg.h>

#include "amiga.h"
#include "stdio.h"

/* Forward declarations */
extern int _scanf(void *ip, int (*get)(), int (*unget)(), const char *fmt, va_list args);

/* Internal function prototypes */
static int fgetc_wrapper(void *ip);
static int ungetc_wrapper(int c, void *ip);
static int sgetc_wrapper(void *ip);
static int sungetc_wrapper(int c, void *ip);

/*
 * fgetc_wrapper - Wrapper for fgetc to match _scanf interface
 */
static int fgetc_wrapper(void *ip)
{
    FILE *fp = (FILE *)ip;
    int c = fgetc(fp);
    return (c == EOF) ? -1 : c;
}

/*
 * ungetc_wrapper - Wrapper for ungetc to match _scanf interface
 */
static int ungetc_wrapper(int c, void *ip)
{
    FILE *fp = (FILE *)ip;
    return ungetc(c, fp);
}

/*
 * scanf - Read formatted input from stdin
 * Supports: %d, %u, %x, %X, %c, %s, %f, %e, %g, %o, %p, %n
 */
int scanf(const char *fmt, ...)
{
    va_list args;
    int result;
    
    va_start(args, fmt);
    result = _scanf(stdin, fgetc_wrapper, ungetc_wrapper, fmt, args);
    va_end(args);
    
    return result;
}

/*
 * vscanf - Read formatted input from stdin with va_list
 * Supports: %d, %u, %x, %X, %c, %s, %f, %e, %g, %o, %p, %n
 */
int vscanf(const char *fmt, va_list args)
{
    return _scanf(stdin, fgetc_wrapper, ungetc_wrapper, fmt, args);
}

/*
 * fscanf - Read formatted input from file stream
 * Supports: %d, %u, %x, %X, %c, %s, %f, %e, %g, %o, %p, %n
 */
int fscanf(FILE *fp, const char *fmt, ...)
{
    va_list args;
    int result;
    
    va_start(args, fmt);
    result = _scanf(fp, fgetc_wrapper, ungetc_wrapper, fmt, args);
    va_end(args);
    
    return result;
}

/*
 * vfscanf - Read formatted input from file stream with va_list
 * Supports: %d, %u, %x, %X, %c, %s, %f, %e, %g, %o, %p, %n
 */
int vfscanf(FILE *fp, const char *fmt, va_list args)
{
    return _scanf(fp, fgetc_wrapper, ungetc_wrapper, fmt, args);
}

/*
 * sscanf - Read formatted input from string
 * Supports: %d, %u, %x, %X, %c, %s, %f, %e, %g, %o, %p, %n
 */
int sscanf(const char *str, const char *fmt, ...)
{
    va_list args;
    int result;
    
    va_start(args, fmt);
    result = _scanf((void *)str, sgetc_wrapper, sungetc_wrapper, fmt, args);
    va_end(args);
    
    return result;
}

/*
 * vsscanf - Read formatted input from string with va_list
 * Supports: %d, %u, %x, %X, %c, %s, %f, %e, %g, %o, %p, %n
 */
int vsscanf(const char *str, const char *fmt, va_list args)
{
    return _scanf((void *)str, sgetc_wrapper, sungetc_wrapper, fmt, args);
}

/*
 * sgetc_wrapper - Wrapper for string getc to match _scanf interface
 */
static int sgetc_wrapper(void *ip)
{
    char **str = (char **)ip;
    char c = **str;
    if (c == '\0') {
        return -1;  /* End of string */
    }
    (*str)++;
    return c;
}

/*
 * sungetc_wrapper - Wrapper for string ungetc to match _scanf interface
 */
static int sungetc_wrapper(int c, void *ip)
{
    char **str = (char **)ip;
    if (*str > (char *)ip) {
        (*str)--;
        return c;
    }
    return -1;  /* Cannot unget at beginning of string */
}

