/*
 * utime.c - set file access and modification times (POSIX compliant)
 *
 * The utime() function sets the access and modification times of the file
 * named by path to the values given in the times structure.
 *
 * POSIX.1-2001, POSIX.1-2008
 */

#include "amiga.h"
#include "timeconvert.h"
#include <stdlib.h>
#include <time.h>
#include <utime.h>
#include <errno.h>

/*
 * utime() - set file access and modification times
 *
 * The utime() function sets the access and modification times of the file
 * named by path to the values given in the times structure.
 *
 * Parameters:
 *   path: pathname of the file whose times to set
 *   times: pointer to utimbuf structure containing times, or NULL for current time
 *
 * Returns: 0 on success, -1 on error with errno set
 *
 * Note: On AmigaOS, only the modification time is actually set due to
 * filesystem limitations. The access time is stored in the times structure
 * but not applied to the file.
 */
int utime(const char *path, const struct utimbuf *times)
{
    struct DateStamp date;
    time_t mtime;

    /* Check for abort signal */
    __chkabort();
    
    /* Validate parameters */
    if (path == NULL) {
        errno = EFAULT;
        return -1;
    }

    /* Determine modification time to use */
    if (times) {
        mtime = times->modtime;
        /* Note: times->actime (access time) is ignored on AmigaOS
         * due to filesystem limitations, but we accept it for POSIX compliance */
    } else {
        /* Use current time if times is NULL */
        mtime = time(NULL);
    }

    /* Check for valid time */
    if (mtime == (time_t)-1) {
        errno = EINVAL;
        return -1;
    }

    /* Convert time to AmigaOS format */
    _gmt2amiga(mtime, &date);

    /* Set the file date */
    if (SetFileDate((char *)path, &date)) {
        return 0;  /* Success */
    }
    
    /* Handle AmigaOS error */
    _seterr();
    return -1;
}
