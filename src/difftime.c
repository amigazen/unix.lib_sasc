/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * difftime.c - calculate difference between two times (POSIX compliant)
 *
 * The difftime() function returns the difference in seconds between
 * time1 and time0 (time1 - time0). The result is returned as a double.
 *
 * POSIX.1-2001, POSIX.1-2008, C99, C11
 */

#include "amiga.h"
#include <time.h>

/*
 * difftime() - calculate difference between two times
 *
 * The difftime() function returns the difference in seconds between
 * time1 and time0 (time1 - time0). The result is returned as a double.
 *
 * Parameters:
 *   time1: end time
 *   time0: start time
 *
 * Returns: difference in seconds as double
 *
 * Note: This is a simple arithmetic operation, but using difftime()
 * ensures proper handling of time_t overflow on systems where time_t
 * is unsigned.
 */
double difftime(time_t time1, time_t time0)
{
    /* Check for abort signal */
    __chkabort();
    
    /* Simple arithmetic difference */
    return (double)(time1 - time0);
}
