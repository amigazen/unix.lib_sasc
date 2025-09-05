/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * pthread_getcpuclockid.c - get CPU-time clock ID of a thread (POSIX compliant)
 *
 * The pthread_getcpuclockid() function returns the clock ID of the CPU-time
 * clock for the thread specified by thread.
 *
 * POSIX.1-2008
 */

#include "amiga.h"
#include <pthread.h>
#include <errno.h>

/* Forward declaration for clockid_t */
typedef int clockid_t;

/* Clock ID constants */
#ifndef CLOCK_THREAD_CPUTIME_ID
#define CLOCK_THREAD_CPUTIME_ID 3
#endif

/*
 * pthread_getcpuclockid() - get CPU-time clock ID of a thread
 *
 * The pthread_getcpuclockid() function returns the clock ID of the CPU-time
 * clock for the thread specified by thread.
 *
 * Parameters:
 *   thread: thread ID
 *   clock_id: pointer to store the clock ID
 *
 * Returns: 0 on success, -1 on error with errno set
 *
 * Note: On AmigaOS, we return CLOCK_THREAD_CPUTIME_ID for the specified
 * thread. CPU time measurement is approximated using system time.
 */
int pthread_getcpuclockid(pthread_t thread, clockid_t *clock_id)
{
    /* Check for abort signal */
    __chkabort();
    
    /* Validate parameters */
    if (clock_id == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    /* Validate thread ID */
    if (thread == 0) {
        errno = EINVAL;
        return -1;
    }
    
    /* For now, we don't have thread-specific CPU time measurement
     * on AmigaOS, so we return the thread CPU-time clock ID but
     * the actual measurement will be approximated using system time */
    *clock_id = CLOCK_THREAD_CPUTIME_ID;
    
    return 0;
}
