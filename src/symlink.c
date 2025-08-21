/*
 * symlink.c - make a symbolic link (POSIX compliant)
 *
 * This function creates a symbolic link named linkpath which contains
 * the string target.
 *
 * POSIX.1-2001, POSIX.1-2008, SVr4, 4.3BSD
 */

#include "amiga.h"
#include <errno.h>
#include <proto/dos.h>
#include <dos/dostags.h>

int symlink(const char *target, const char *linkpath)
{
    chkabort();
    
    if (target == NULL || linkpath == NULL) {
        errno = EFAULT;
        return -1;
    }
    
    /* Create the symbolic link */
    if (MakeLink(linkpath, (LONG)target, TRUE)) {
        return 0;
    } else {
        errno = convert_oserr(IoErr());
        return -1;
    }
}
