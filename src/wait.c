/*
 * wait.c - wait for child process termination (POSIX compliant)
 *
 * This function suspends execution of the calling thread until
 * one of its children terminates. On AmigaOS, this returns ENOSYS
 * as there's no fork/exec system.
 *
 * POSIX.1-2001, POSIX.1-2008, SVr4, 4.3BSD
 */

#include "amiga.h"
#include <sys/wait.h>
#include <errno.h>

pid_t wait(int *status)
{
    chkabort();
    
    /* On AmigaOS, there's no fork/exec system, so this is not supported */
    errno = ENOSYS;
    return -1;
}
