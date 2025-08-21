/*
 * tzset.c - initialize timezone information (POSIX compliant)
 *
 * This function initializes the tzname variable from the TZ environment
 * variable. This function is automatically called by the other time
 * conversion functions that depend on timezone information.
 *
 * POSIX.1-2001, POSIX.1-2008, SVr4, 4.3BSD
 */

#include "amiga.h"
#include <time.h>
#include <stdlib.h>

void tzset(void)
{
    chkabort();
    
    /* On AmigaOS, timezone handling is simplified */
    /* The ctime.c module handles most timezone conversion */
    
    /* This function is a no-op for now, but could be extended
       to handle TZ environment variable parsing */
}
