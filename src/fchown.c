/*
 * fchown.c - change file ownership by file descriptor (POSIX compliant)
 *
 * This function changes the owner and group of the file given by fd.
 * On AmigaOS, this is a no-op as there's no ownership system.
 *
 * POSIX.1-2001, POSIX.1-2008, SVr4, 4.3BSD
 */

#include "amiga.h"
#include <unistd.h>
#include <errno.h>
#include "files.h"

int fchown(int fd, uid_t owner, gid_t group)
{
    struct fileinfo *fi;
    
    chkabort();
    
    /* Check if file descriptor is valid */
    if ((fi = _find_fd(fd)) == NULL) {
        errno = EBADF;
        return -1;
    }
    
    /* On AmigaOS, there's no ownership system, so this is a no-op */
    return 0;
}
