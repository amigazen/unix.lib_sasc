/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * clock_getcpuclockid.c - get CPU-time clock ID of a process (POSIX compliant)
 *
 * The clock_getcpuclockid() function returns the clock ID of the CPU-time
 * clock for the process specified by pid.
 *
 * POSIX.1-2008
 */

#include "amiga.h"
#include <sys/timer.h>
#include <errno.h>
#include <unistd.h>

/*
 * clock_getcpuclockid() - get CPU-time clock ID of a process
 *
 * The clock_getcpuclockid() function returns the clock ID of the CPU-time
 * clock for the process specified by pid.
 *
 * Parameters:
 *   pid: process ID (0 for current process)
 *   clock_id: pointer to store the clock ID
 *
 * Returns: 0 on success, -1 on error with errno set
 *
 * Note: On AmigaOS, we return CLOCK_PROCESS_CPUTIME_ID for the current
 * process. CPU time measurement is approximated using system time.
 */
int clock_getcpuclockid(pid_t pid, clockid_t *clock_id)
{
    /* Check for abort signal */
    __chkabort();
    
    /* Validate parameters */
    if (clock_id == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    /* Only support current process (pid = 0) */
    if (pid != 0) {
        /* Check if pid is current process */
        if (pid != getpid()) {
            errno = ESRCH;  /* Process not found */
            return -1;
        }
    }
    
    /* Return CPU-time clock ID for current process */
    *clock_id = CLOCK_PROCESS_CPUTIME_ID;
    
    return 0;
}

