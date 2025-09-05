#include "amiga.h"
#include "files.h"
#include <fcntl.h>

/* Internal implementation */
int __read(int fd, void *buffer, unsigned int length)
{
    struct fileinfo *fi;

    __chkabort();
    if (fi = _find_fd(fd)) {
	if (fi->flags & FI_READ)
	    return fi->read(fi->userinfo, buffer, length);
	errno = EACCES;
    }
    return -1;
}

/* Public POSIX read function */
int read(int fd, void *buffer, unsigned int length)
{
    return __read(fd, buffer, length);
}
