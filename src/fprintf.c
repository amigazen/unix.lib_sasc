/*
 * Enhanced fprintf implementation
 * 
 * SPDX-License-Identifier: BSD-2-Clause
 * Copyright (c) 2025 amigazen project
 */

#include <stdio.h>
#include <stdarg.h>

#include "amiga.h"
#include "stdio.h"

/* Forward declaration */
extern int _printf(void *op, int (*put)(), const char *fmt, va_list args);

/*
 * aputc - AmigaOS putc function for fprintf
 */
static int aputc(int c, void *op)
{
    FILE *fp = (FILE *)op;
    return fputc(c, fp);
}

/*
 * fprintf - Print formatted output to file stream
 * Supports all format specifiers from _printf
 */
int fprintf(FILE *fp, const char *fmt, ...)
{
    va_list args;
    int result;
    
    va_start(args, fmt);
    result = _printf(fp, aputc, fmt, args);
    va_end(args);
    
    return result;
}

/*
 * vfprintf - Print formatted output to file stream with va_list
 * Supports all format specifiers from _printf
 */
int vfprintf(FILE *fp, const char *fmt, va_list args)
{
    return _printf(fp, aputc, fmt, args);
}

/*
 * printf - Print formatted output to stdout
 * Supports all format specifiers from _printf
 */
int printf(const char *fmt, ...)
{
    va_list args;
    int result;
    
    va_start(args, fmt);
    result = _printf(stdout, aputc, fmt, args);
    va_end(args);
    
    return result;
}

/*
 * vprintf - Print formatted output to stdout with va_list
 * Supports all format specifiers from _printf
 */
int vprintf(const char *fmt, va_list args)
{
    return _printf(stdout, aputc, fmt, args);
}

