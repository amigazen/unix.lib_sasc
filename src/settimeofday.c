/*
 * settimeofday.c - set system time (POSIX compliant)
 *
 * This function sets the system's idea of the current time. The
 * time is expressed in seconds and microseconds since midnight
 * (0 hour), January 1, 1970.
 *
 * POSIX.1-2001, POSIX.1-2008, SVr4, 4.3BSD
 */

#include "amiga.h"
#include <sys/time.h>
#include <errno.h>
#include <proto/dos.h>
#include <dos/dostags.h>

int settimeofday(const struct timeval *tv, const struct timezone *tz)
{
    chkabort();
    
    if (tv == NULL) {
        errno = EFAULT;
        return -1;
    }
    
    /* On AmigaOS, we can't easily set the system time */
    /* This would require root privileges and system calls */
    errno = EPERM;
    return -1;
}
