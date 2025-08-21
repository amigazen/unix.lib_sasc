/*
 * dup2.c - duplicate a file descriptor to a specific number (POSIX compliant)
 *
 * This function duplicates an existing file descriptor to a specific
 * file descriptor number, closing the target if it's already open.
 *
 * POSIX.1-2001, POSIX.1-2008, SVr4, 4.3BSD
 */

#include "amiga.h"
#include "files.h"
#include <errno.h>
#include <fcntl.h>

int dup2(int oldfd, int newfd)
{
    struct fileinfo *fi;
    
    chkabort();
    
    /* Check if there's nothing to do */
    if (oldfd == newfd) {
        return newfd;
    }
    
    /* Check if oldfd is valid */
    if ((fi = _find_fd(oldfd)) == NULL) {
        errno = EBADF;
        return -1;
    }
    
    /* Check if newfd is valid */
    if (newfd < 0) {
        errno = EBADF;
        return -1;
    }
    
    /* Close newfd if it's already open */
    if (_find_fd(newfd) != NULL) {
        close(newfd);
    }
    
    /* Allocate the new file descriptor */
    if (_alloc_fd_at(fi->userinfo, fi->flags, fi->select_start, 
                     fi->select_poll, fi->read, fi->write, 
                     fi->lseek, fi->close, fi->ioctl, newfd) < 0) {
        errno = EMFILE;  /* Too many open files */
        return -1;
    }
    
    return newfd;
}
