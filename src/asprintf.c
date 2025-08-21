/*
 * asprintf.c - allocate and format string (POSIX compliant)
 *
 * This function allocates a string large enough to hold the output
 * including the terminating null byte, and returns a pointer to it.
 * The pointer should be passed to free() to release the allocated
 * storage when it is no longer needed.
 *
 * POSIX.1-2001, POSIX.1-2008, 4.3BSD
 */

#include "amiga.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>

int asprintf(char **strp, const char *fmt, ...)
{
    va_list args;
    int result;
    
    chkabort();
    
    if (strp == NULL || fmt == NULL) {
        errno = EFAULT;
        return -1;
    }
    
    va_start(args, fmt);
    result = vasprintf(strp, fmt, args);
    va_end(args);
    
    return result;
}
