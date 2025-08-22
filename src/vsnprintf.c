#include <stdarg.h>
#include <stddef.h> // For size_t

#include "amiga.h"  // For ULONG, UBYTE, APTR, VSNPrintf, LONG

/*
 * Modern POSIX-compatible vsnprintf() function implementation for AmigaOS.
 * This function formats output according to the format string and writes
 * up to bufsize-1 characters to the buffer, ensuring NUL termination.
 * * Returns the number of characters that would have been written if
 * bufsize had been sufficiently large, not counting the terminating NUL.
 * This matches the C99 and POSIX standards.
 */
int vsnprintf(char *buffer, size_t bufsize, const char *fmt, va_list args)
{
    LONG needed_size_incl_nul;
    
    /* According to the POSIX standard, if bufsize is 0, buffer can be NULL
       and nothing is written. The function should still return the number of
       characters that would have been written. We let VSNPrintf handle this. */

    /* A NULL buffer is only valid if bufsize is 0. */
    if (buffer == NULL && bufsize != 0) {
        return -1;
    }

    /* Call the AmigaOS native VSNPrintf.
       It's assumed to return the total number of bytes required for the output,
       *including* the terminating NUL character.
       It returns 0 or a negative value on error. */
    needed_size_incl_nul = VSNPrintf((UBYTE *)buffer, (ULONG)bufsize, (const UBYTE *)fmt, (APTR)args);

    /* VSNPrintf returns a value <= 0 on error. We return -1 as is standard. */
    if (needed_size_incl_nul <= 0) {
        return -1;
    }

    /* The POSIX standard requires returning the length of the string
       that *would* have been written, *excluding* the NUL terminator.
       This is simply the value VSNPrintf gave us, minus one. */
    return (int)(needed_size_incl_nul - 1);
}
