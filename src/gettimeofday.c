/*
 * gettimeofday.c - get time of day (POSIX compliant)
 *
 * This function gets the system's notion of the current Greenwich time
 * and the current time zone. The time is expressed in seconds and
 * microseconds since midnight (0 hour), January 1, 1970.
 *
 * POSIX.1-2001, POSIX.1-2008, SVr4, 4.3BSD
 */

#include "amiga.h"
#include <sys/time.h>
#include <time.h>
#include <errno.h>

/* External timezone information */
extern struct timezone __time_zone;
extern long __local_to_GMT;

int gettimeofday(struct timeval *tp, struct timezone *tzp)
{
    chkabort();
    
    if (tp) {
        /* Get current system time */
        time_t now = time(NULL);
        tp->tv_sec = now + __local_to_GMT;
        tp->tv_usec = 0;  /* AmigaOS doesn't provide microsecond precision */
    }
    
    if (tzp) {
        /* Return timezone information */
        *tzp = __time_zone;
    }
    
    return 0;
}
