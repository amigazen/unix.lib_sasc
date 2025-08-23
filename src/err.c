/*
 * err.c - error and warning reporting (POSIX compliant)
 *
 * This file contains the err() and warn() family of functions that
 * provide formatted error and warning messages.
 *
 * POSIX.1-2001, POSIX.1-2008, 4.3BSD
 */

#include "amiga.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>

void err(int eval, const char *fmt, ...)
{
    va_list args;
    
    chkabort();
    
    fprintf(stderr, "%s: ", "program");
    
    if (fmt != NULL) {
        va_start(args, fmt);
        vfprintf(stderr, fmt, args);
        va_end(args);
    }
    
    if (errno != 0) {
        fprintf(stderr, ": %s", strerror(errno));
    }
    
    fprintf(stderr, "\n");
    
    exit(eval);
}

void errx(int eval, const char *fmt, ...)
{
    va_list args;
    
    chkabort();
    
    fprintf(stderr, "%s: ", "program");
    
    if (fmt != NULL) {
        va_start(args, fmt);
        vfprintf(stderr, fmt, args);
        va_end(args);
    }
    
    fprintf(stderr, "\n");
    
    exit(eval);
}

void verr(int eval, const char *fmt, va_list args)
{
    chkabort();
    
    fprintf(stderr, "%s: ", "program");
    
    if (fmt != NULL) {
        vfprintf(stderr, fmt, args);
    }
    
    if (errno != 0) {
        fprintf(stderr, ": %s", strerror(errno));
    }
    
    fprintf(stderr, "\n");
    
    exit(eval);
}

void verrx(int eval, const char *fmt, va_list args)
{
    chkabort();
    
    fprintf(stderr, "%s: ", "program");
    
    if (fmt != NULL) {
        vfprintf(stderr, fmt, args);
    }
    
    fprintf(stderr, "\n");
    
    exit(eval);
}

void warn(const char *fmt, ...)
{
    va_list args;
    
    chkabort();
    
    fprintf(stderr, "%s: ", "program");
    
    if (fmt != NULL) {
        va_start(args, fmt);
        vfprintf(stderr, fmt, args);
        va_end(args);
    }
    
    if (errno != 0) {
        fprintf(stderr, ": %s", strerror(errno));
    }
    
    fprintf(stderr, "\n");
}

void warnx(const char *fmt, ...)
{
    va_list args;
    
    chkabort();
    
    fprintf(stderr, "%s: ", "program");
    
    if (fmt != NULL) {
        va_start(args, fmt);
        vfprintf(stderr, fmt, args);
        va_end(args);
    }
    
    fprintf(stderr, "\n");
}

void vwarn(const char *fmt, va_list args)
{
    chkabort();
    
    fprintf(stderr, "%s: ", "program");
    
    if (fmt != NULL) {
        vfprintf(stderr, fmt, args);
    }
    
    if (errno != 0) {
        fprintf(stderr, ": %s", strerror(errno));
    }
    
    fprintf(stderr, "\n");
}

void vwarnx(const char *fmt, va_list args)
{
    chkabort();
    
    fprintf(stderr, "%s: ", "program");
    
    if (fmt != NULL) {
        vfprintf(stderr, fmt, args);
    }
    
    fprintf(stderr, "\n");
}
