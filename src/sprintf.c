/*
 * Enhanced sprintf implementation
 * 
 * SPDX-License-Identifier: BSD-2-Clause
 * Copyright (c) 2025 amigazen project
 */

#include <stdarg.h>
#include <string.h>

#include "amiga.h"
#include "stdio.h"

/* Forward declaration */
extern int _printf(void *op, int (*put)(), const char *fmt, va_list args);

/*
 * sputc - String putc function for sprintf
 */
static int sputc(int c, void *op)
{
    char **s = (char **)op;
    *(*s)++ = c;
    return c;
}

/*
 * sprintf - Print formatted output to string buffer
 * Supports all format specifiers from _printf
 * Note: This function does not check buffer bounds - use snprintf for safety
 */
int sprintf(char *buf, const char *fmt, ...)
{
    va_list args;
    int n;
    char *p = buf;
    
    va_start(args, fmt);
    n = _printf(&p, sputc, fmt, args);
    va_end(args);
    
    *p = '\0';  /* Always null-terminate the string */
    return n;
}

/*
 * vsprintf - Print formatted output to string buffer with va_list
 * Supports all format specifiers from _printf
 * Note: This function does not check buffer bounds - use vsnprintf for safety
 */
int vsprintf(char *buf, const char *fmt, va_list args)
{
    char *p = buf;
    int n;
    
    n = _printf(&p, sputc, fmt, args);
    *p = '\0';  /* Always null-terminate the string */
    
    return n;
}

