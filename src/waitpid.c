/*
 * waitpid.c - wait for specific child process termination (POSIX compliant)
 *
 * This function suspends execution of the calling thread until
 * a child as specified by the pid argument has changed state.
 * On AmigaOS, this returns ENOSYS as there's no fork/exec system.
 *
 * POSIX.1-2001, POSIX.1-2008, SVr4, 4.3BSD
 */

#include "amiga.h"
#include <sys/wait.h>
#include <errno.h>

pid_t waitpid(pid_t pid, int *status, int options)
{
    chkabort();
    
    /* On AmigaOS, there's no fork/exec system, so this is not supported */
    errno = ENOSYS;
    return -1;
}
