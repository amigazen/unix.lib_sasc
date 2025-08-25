#include <stdarg.h>
#include <stddef.h>  // For size_t

#include "amiga.h"  // For LONG, UBYTE, SNPrintf

/*
 * Modern POSIX-compatible snprintf() function implementation for AmigaOS.
 * This function formats output according to the format string and writes
 * up to bufsize-1 characters to the buffer, ensuring NUL termination.
 * 
 * Returns the number of characters that would have been written if
 * bufsize had been sufficiently large, not counting the terminating NUL.
 */

int snprintf(char *buffer, size_t bufsize, const char *fmt, ...)
{
    va_list args;
    LONG needed_size_incl_nul;
    
    /* Check for valid parameters */
    if (buffer == NULL && bufsize != 0) {
        return -1;
    }
    
    /* Initialize variable argument list */
    va_start(args, fmt);
    
    /* Call AmigaOS native SNPrintf with va_list.
       It returns the total number of bytes required for the output,
       *including* the terminating NUL character.
       It returns 0 or a negative value on error. */
    needed_size_incl_nul = VSNPrintf((UBYTE *)buffer, (ULONG)bufsize, (const UBYTE *)fmt, (APTR)args);
    
    /* Clean up variable argument list */
    va_end(args);
    
    /* SNPrintf returns a value <= 0 on error. We return -1 as is standard. */
    if (needed_size_incl_nul <= 0) {
        return -1;
    }
    
    /* The POSIX standard requires returning the length of the string
       that *would* have been written, *excluding* the NUL terminator.
       This is simply the value SNPrintf gave us, minus one. */
    return (int)(needed_size_incl_nul - 1);
}
