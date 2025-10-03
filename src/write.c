#include "amiga.h"
#include "files.h"
#include <fcntl.h>
#include <unistd.h>
#include <stddef.h>

/* Internal implementation */
ssize_t __write(int fd, const void *buffer, size_t length)
{
    struct fileinfo *fi;

    __chkabort();
    if (fi = _find_fd(fd)) {
	if (fi->flags & FI_WRITE) {
	    if (fi->flags & O_APPEND)
		fi->lseek(fi->userinfo, 0, SEEK_END);
	    return fi->write(fi->userinfo, (void *) buffer, length);
	}
	errno = EACCES;
    }
    return -1;
}

/* Public POSIX write function */
ssize_t write(int fd, const void *buffer, size_t length)
{
    return __write(fd, buffer, length);
}
