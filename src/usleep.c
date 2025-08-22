/*
 * usleep.c - suspend execution for microsecond intervals (POSIX compliant)
 *
 * This function suspends the execution of the calling thread for
 * (at least) usec microseconds. The sleep may be lengthened slightly
 * by any system activity or by the time spent processing the call.
 *
 * POSIX.1-2001, POSIX.1-2008, SVr4, 4.3BSD
 */

#include "amiga.h"
#include <unistd.h>
#include <errno.h>
#include <proto/dos.h>

int usleep(unsigned int usec)
{    
    /* Convert microseconds to ticks (1/50 second on PAL, 1/60 on NTSC) */
    /* Use 1/50 as default for compatibility */
    unsigned int ticks = (usec * 50) / 1000000;
    
    __chkabort();

    /* Ensure at least 1 tick if usec > 0 */
    if (usec > 0 && ticks == 0) {
        ticks = 1;
    }
    
    /* Sleep using AmigaOS Delay() function */
    Delay(ticks);
    
    return 0;
}
