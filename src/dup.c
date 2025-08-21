/*
 * dup.c - duplicate a file descriptor (POSIX compliant)
 *
 * This function duplicates an existing file descriptor and returns
 * a new file descriptor that refers to the same open file description.
 *
 * POSIX.1-2001, POSIX.1-2008, SVr4, 4.3BSD
 */

#include "amiga.h"
#include "files.h"
#include <errno.h>
#include <fcntl.h>

int dup(int oldfd)
{
    struct fileinfo *fi;
    int newfd;
    
    chkabort();
    
    /* Check if oldfd is valid */
    if ((fi = _find_fd(oldfd)) == NULL) {
        errno = EBADF;
        return -1;
    }
    
    /* Allocate a new file descriptor */
    newfd = _alloc_fd(fi->userinfo, fi->flags, fi->select_start, 
                      fi->select_poll, fi->read, fi->write, 
                      fi->lseek, fi->close, fi->ioctl);
    
    if (newfd < 0) {
        errno = EMFILE;  /* Too many open files */
        return -1;
    }
    
    return newfd;
}
