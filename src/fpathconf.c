/*
 * fpathconf.c - get file path configuration values (POSIX compliant)
 *
 * This function provides a method for applications to determine the
 * current value of a configurable limit or option variable that is
 * associated with a file descriptor.
 *
 * POSIX.1-2001, POSIX.1-2008, SVr4, 4.3BSD
 */

#include "amiga.h"
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include "files.h"

/* Path configuration constants */
#define _PC_LINK_MAX          1
#define _PC_MAX_CANON         2
#define _PC_MAX_INPUT         3
#define _PC_NAME_MAX          4
#define _PC_PATH_MAX          5
#define _PC_PIPE_BUF          6
#define _PC_CHOWN_RESTRICTED  7
#define _PC_NO_TRUNC          8
#define _PC_VDISABLE          9

long fpathconf(int fd, int name)
{
    struct fileinfo *fi;
    
    chkabort();
    
    /* Check if file descriptor is valid */
    if ((fi = _find_fd(fd)) == NULL) {
        errno = EBADF;
        return -1;
    }
    
    switch (name) {
        case _PC_LINK_MAX:
            return 1;      /* AmigaOS supports hard links */
            
        case _PC_MAX_CANON:
            return -1;     /* Not applicable on AmigaOS */
            
        case _PC_MAX_INPUT:
            return -1;     /* Not applicable on AmigaOS */
            
        case _PC_NAME_MAX:
            return 107;    /* AmigaOS filename limit */
            
        case _PC_PATH_MAX:
            return 256;    /* Reasonable path limit */
            
        case _PC_PIPE_BUF:
            return 4096;   /* Reasonable pipe buffer size */
            
        case _PC_CHOWN_RESTRICTED:
            return 1;      /* chown is restricted on AmigaOS */
            
        case _PC_NO_TRUNC:
            return 0;      /* Names are truncated on AmigaOS */
            
        case _PC_VDISABLE:
            return 0;      /* Not applicable on AmigaOS */
            
        default:
            errno = EINVAL;
            return -1;
    }
}
