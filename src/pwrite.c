/*
 * pwrite.c - write to file at specific offset (POSIX compliant)
 *
 * This function writes up to count bytes from the buffer starting
 * at buf to the file descriptor fd at offset offset. The file
 * offset is not changed.
 *
 * POSIX.1-2001, POSIX.1-2008, SVr4, 4.3BSD
 */

#include "amiga.h"
#include <unistd.h>
#include <errno.h>
#include "files.h"

ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset)
{
    struct fileinfo *fi;
    off_t old_offset;
    ssize_t result;
    
    chkabort();
    
    if (buf == NULL) {
        errno = EFAULT;
        return -1;
    }
    
    /* Check if file descriptor is valid */
    if ((fi = _find_fd(fd)) == NULL) {
        errno = EBADF;
        return -1;
    }
    
    /* Save current offset */
    old_offset = lseek(fd, 0, SEEK_CUR);
    if (old_offset == -1) {
        return -1;
    }
    
    /* Seek to specified offset */
    if (lseek(fd, offset, SEEK_SET) == -1) {
        return -1;
    }
    
    /* Write the data */
    result = write(fd, buf, count);
    
    /* Restore original offset */
    lseek(fd, old_offset, SEEK_SET);
    
    return result;
}
