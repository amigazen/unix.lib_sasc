#include "amiga.h"
#include "files.h"
#include <fcntl.h>
#include <unistd.h>

/* Internal implementation */
int __write(int fd, const void *buffer, unsigned int length)
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
int write(int fd, const void *buffer, unsigned int length)
{
    return __write(fd, buffer, length);
}
