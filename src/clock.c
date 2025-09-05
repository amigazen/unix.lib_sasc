/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * clock.c - measure program processor time (POSIX compliant)
 *
 * The clock() function returns an approximation of the amount of
 * processor time used by the program, relative to the base point.
 * The base point is the time the program's startup code was run.
 *
 * POSIX.1-2001, POSIX.1-2008, C99, C11
 */

#include "amiga.h"
#include <time.h>

#ifndef CLOCKS_PER_SEC
#define CLOCKS_PER_SEC 1000
#endif

/* Static variable to store program start time */
static time_t program_start_time = 0;
static int clock_initialized = 0;

/*
 * clock() - measure program processor time
 *
 * The clock() function returns an approximation of the amount of
 * processor time used by the program, relative to the base point.
 * The base point is the time the program's startup code was run.
 *
 * Returns: processor time used in clock ticks, or (clock_t)-1 on error
 *
 * Note: On Amiga, we approximate processor time by using elapsed
 * system time since program start. This is not true CPU time but
 * provides a reasonable approximation for timing purposes.
 */
clock_t clock(void)
{
    time_t current_time;
    time_t elapsed_time;
    
    /* Check for abort signal */
    __chkabort();
    
    /* Initialize start time on first call */
    if (!clock_initialized) {
        program_start_time = time(NULL);
        if (program_start_time == (time_t)-1) {
            return (clock_t)-1;
        }
        clock_initialized = 1;
    }
    
    /* Get current time */
    current_time = time(NULL);
    if (current_time == (time_t)-1) {
        return (clock_t)-1;
    }
    
    /* Calculate elapsed time since program start */
    elapsed_time = current_time - program_start_time;
    
    /* Convert to clock ticks (CLOCKS_PER_SEC = 1000) */
    return (clock_t)(elapsed_time * CLOCKS_PER_SEC);
}
