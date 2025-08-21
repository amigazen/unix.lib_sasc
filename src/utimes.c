/*
 * utimes.c - set file access and modification times (POSIX compliant)
 *
 * This function sets the access and modification times of the file
 * named by filename to the values given in the times array.
 *
 * POSIX.1-2001, POSIX.1-2008, 4.3BSD
 */

#include "amiga.h"
#include <sys/time.h>
#include <errno.h>
#include <proto/dos.h>
#include <dos/dostags.h>

int utimes(const char *filename, const struct timeval times[2])
{
    struct utimbuf ut;
    
    chkabort();
    
    if (filename == NULL) {
        errno = EFAULT;
        return -1;
    }
    
    if (times == NULL) {
        /* Set current time if times is NULL */
        return utime(filename, NULL);
    }
    
    /* Convert timeval to utimbuf */
    ut.actime = times[0].tv_sec;
    ut.modtime = times[1].tv_sec;
    
    /* Call utime() to do the actual work */
    return utime(filename, &ut);
}
