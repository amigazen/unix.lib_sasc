/*
 * fork.c - create a new process (POSIX compliant)
 *
 * This function creates a new process by duplicating the calling process.
 * On AmigaOS, this returns ENOSYS as there's no fork system.
 *
 * POSIX.1-2001, POSIX.1-2008, SVr4, 4.3BSD
 */

#include "amiga.h"
#include <unistd.h>
#include <errno.h>

pid_t fork(void)
{
    chkabort();
    
    /* On AmigaOS, there's no fork system, so this is not supported */
    errno = ENOSYS;
    return -1;
}
