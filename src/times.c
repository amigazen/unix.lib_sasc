/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * times.c - get process times (POSIX compliant)
 *
 * The times() function returns the current process times in the structure
 * pointed to by buffer. The times returned are measured in clock ticks.
 *
 * POSIX.1-2001, POSIX.1-2008, SVr4, 4.3BSD
 */

#include "amiga.h"
#include <sys/times.h>
#include <time.h>
#include <errno.h>

/* Static variables to track process timing */
static time_t process_start_time = 0;
static int times_initialized = 0;

/* Child process timing (simplified - AmigaOS doesn't track child CPU time) */
static clock_t child_user_time = 0;
static clock_t child_system_time = 0;

/*
 * times() - get process times
 *
 * The times() function fills the tms structure pointed to by buffer
 * with time-accounting information. The times returned are measured
 * in clock ticks.
 *
 * Parameters:
 *   buffer: pointer to tms structure to fill
 *
 * Returns: elapsed real time in clock ticks, or (clock_t)-1 on error
 *
 * Note: On Amiga, we approximate CPU time using elapsed system time
 * since process start. This is not true CPU time but provides a
 * reasonable approximation for timing purposes.
 */
clock_t times(struct tms *buffer)
{
    time_t current_time;
    time_t elapsed_time;
    clock_t elapsed_ticks;
    
    /* Check for abort signal */
    __chkabort();
    
    /* Validate parameter */
    if (buffer == NULL) {
        errno = EINVAL;
        return (clock_t)-1;
    }
    
    /* Initialize start time on first call */
    if (!times_initialized) {
        process_start_time = time(NULL);
        if (process_start_time == (time_t)-1) {
            return (clock_t)-1;
        }
        times_initialized = 1;
    }
    
    /* Get current time */
    current_time = time(NULL);
    if (current_time == (time_t)-1) {
        return (clock_t)-1;
    }
    
    /* Calculate elapsed time since process start */
    elapsed_time = current_time - process_start_time;
    
    /* Convert to clock ticks (CLK_TCK = 60) */
    elapsed_ticks = (clock_t)(elapsed_time * CLK_TCK);
    
    /* Fill the tms structure */
    /* On AmigaOS, we approximate user time as 80% of elapsed time */
    buffer->tms_utime = (clock_t)(elapsed_ticks * 0.8);
    
    /* System time as 20% of elapsed time */
    buffer->tms_stime = (clock_t)(elapsed_ticks * 0.2);
    
    /* Child process times (simplified - Amiga doesn't track these) */
    buffer->tms_cutime = child_user_time;
    buffer->tms_cstime = child_system_time;
    
    /* Return elapsed real time in clock ticks */
    return elapsed_ticks;
}
