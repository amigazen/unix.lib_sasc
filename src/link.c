/*
 * link.c - make a hard link to a file (POSIX compliant)
 *
 * This function creates a new link (directory entry) for the existing file
 * named by path1. The new link is named by path2.
 *
 * POSIX.1-2001, POSIX.1-2008, SVr4, 4.3BSD
 */

#include "amiga.h"
#include <errno.h>
#include <proto/dos.h>
#include <dos/dostags.h>

int link(const char *path1, const char *path2)
{
    BPTR lock1, lock2;
    
    chkabort();
    
    if (path1 == NULL || path2 == NULL) {
        errno = EFAULT;
        return -1;
    }
    
    /* Lock the source file */
    lock1 = Lock(path1, SHARED_LOCK);
    if (lock1 == NULL) {
        errno = convert_oserr(IoErr());
        return -1;
    }
    
    /* Create the hard link */
    if (MakeLink(path2, (LONG)lock1, FALSE)) {
        UnLock(lock1);
        return 0;
    } else {
        UnLock(lock1);
        errno = convert_oserr(IoErr());
        return -1;
    }
}
