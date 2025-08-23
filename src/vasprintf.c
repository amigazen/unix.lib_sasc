/*
 * vasprintf.c - allocate and format string with va_list (POSIX compliant)
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

int vasprintf(char **strp, const char *fmt, va_list args)
{
    va_list args_copy;
    int size;
    char *buffer;
    
    chkabort();
    
    if (strp == NULL || fmt == NULL) {
        errno = EFAULT;
        return -1;
    }
    
    /* Make a copy of args since vsnprintf might modify it */
    va_copy(args_copy, args);
    
    /* First, determine the required size */
    size = vsnprintf(NULL, 0, fmt, args_copy);
    va_end(args_copy);
    
    if (size < 0) {
        return -1;
    }
    
    /* Allocate buffer (add 1 for null terminator) */
    buffer = malloc(size + 1);
    if (buffer == NULL) {
        errno = ENOMEM;
        return -1;
    }
    
    /* Format the string */
    size = vsnprintf(buffer, size + 1, fmt, args);
    if (size < 0) {
        free(buffer);
        return -1;
    }
    
    *strp = buffer;
    return size;
}
