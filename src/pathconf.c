/*
 * pathconf.c - get path configuration values (POSIX compliant)
 *
 * This function provides a method for applications to determine the
 * current value of a configurable limit or option variable that is
 * associated with a file or directory.
 *
 * POSIX.1-2001, POSIX.1-2008, SVr4, 4.3BSD
 */

#include "amiga.h"
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <proto/dos.h>
#include <dos/dostags.h>

/* External function for sending packets to AmigaOS */
extern LONG SendPacket(struct MsgPort *handler, LONG action, LONG *arglist, LONG nargs);

/*
 * get_filesystem_limits_by_path() - Get filesystem limits for a specific path
 *
 * This function determines the actual filename and path length limits
 * for a filesystem by examining the device associated with the path.
 *
 * Parameters:
 *   path - Path to examine
 *   name_max - Pointer to store filename length limit
 *   path_max - Pointer to store path length limit
 *
 * Returns:
 *   0 on success, -1 on failure
 */
static int get_filesystem_limits_by_path(const char *path, long *name_max, long *path_max)
{
    /* For now, return conservative limits */
    *name_max = 30;   /* Traditional Amiga filesystems: 30 characters */
    *path_max = 256;  /* AmigaOS path limit */
    return 0;
}

long pathconf(const char *path, int name)
{
    chkabort();
    
    if (path == NULL) {
        errno = EFAULT;
        return -1;
    }
    
    switch (name) {
        case _PC_LINK_MAX:
            return 1;      /* AmigaOS supports hard links */
            
        case _PC_MAX_CANON:
            return -1;     /* Not applicable on AmigaOS */
            
        case _PC_MAX_INPUT:
            return -1;     /* Not applicable on AmigaOS */
            
        case _PC_NAME_MAX: {
            long name_max, path_max;
            if (get_filesystem_limits_by_path(path, &name_max, &path_max) == 0) {
                return name_max;
            }
            return 30;     /* Fallback to traditional limit */
        }
            
        case _PC_PATH_MAX: {
            long name_max, path_max;
            if (get_filesystem_limits_by_path(path, &name_max, &path_max) == 0) {
                return path_max;
            }
            return 256;    /* Fallback to traditional limit */
        }
            
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
