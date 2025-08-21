/*
 * setpgid.c - set process group ID (POSIX compliant)
 *
 * This function sets the process group ID of the process specified
 * by pid to pgid. On AmigaOS, this is a no-op as there's no
 * process group system.
 *
 * POSIX.1-2001, POSIX.1-2008, SVr4, 4.3BSD
 */

#include "amiga.h"
#include <unistd.h>
#include <errno.h>

int setpgid(pid_t pid, pid_t pgid)
{
    chkabort();
    
    /* On AmigaOS, there's no process group system, so this is a no-op */
    /* Return success to maintain compatibility */
    return 0;
}
