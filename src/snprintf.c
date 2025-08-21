#include <stdarg.h>
#include <stddef.h>  // For size_t

#include "amiga.h"  // For LONG (AmigaOS type)

/*
 * Modern POSIX-compatible snprintf() function implementation for AmigaOS.
 * This function formats output according to the format string and writes
 * up to bufsize-1 characters to the buffer, ensuring NUL termination.
 * 
 * Returns the number of characters that would have been written if
 * bufsize had been sufficiently large, not counting the terminating NUL.
 */

extern int vsnprintf(char *buffer, size_t bufsize, const char *fmt, va_list args);

int snprintf(char *buffer, size_t bufsize, const char *fmt, ...)
{
    va_list args;
    LONG result;
    
    /* Check for valid parameters */
    if (buffer == NULL || bufsize == 0) {
        return 0;
    }
    
    /* Initialize variable argument list */
    va_start(args, fmt);
    
    /* Call vsnprintf with the variable arguments */
    result = vsnprintf(buffer, bufsize, fmt, args);
    
    /* Clean up variable argument list */
    va_end(args);
    
    return result;
}
