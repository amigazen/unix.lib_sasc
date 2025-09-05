#include "amiga.h"
#include "files.h"
#include <fcntl.h>

/* Internal implementation */
int __lseek(int fd, long rpos, int mode)
{
    struct fileinfo *fi;

    __chkabort();
    if (fi = _find_fd(fd)) {
	return fi->lseek(fi->userinfo, rpos, mode);
    }
    return -1;
}

/* Public POSIX lseek function */
int lseek(int fd, long rpos, int mode)
{
    return __lseek(fd, rpos, mode);
}
