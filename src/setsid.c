/*
 * setsid.c - create session and set process group ID (POSIX compliant)
 *
 * This function creates a new session if the calling process is not
 * already a process group leader. On AmigaOS, this is a no-op as
 * there's no session system.
 *
 * POSIX.1-2001, POSIX.1-2008, SVr4, 4.3BSD
 */

#include "amiga.h"
#include <unistd.h>
#include <errno.h>

pid_t setsid(void)
{
    chkabort();
    
    /* On AmigaOS, there's no session system, so this is a no-op */
    /* Return the current process ID to maintain compatibility */
    return getpid();
}
