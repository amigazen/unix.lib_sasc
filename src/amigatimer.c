/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * amigatimer.c - Amiga timer.device integration for fine-grained timing
 *
 * This file provides helper functions for using the Amiga timer.device
 * for high-precision timing operations.
 *
 * Based on Amiga RKM Timer Device documentation
 */

#include "amiga.h"
#include "amigatimer.h"
#include "longlong.h"
#include <devices/timer.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/timer.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

/* Timer device management structure */
struct timer_device {
    struct MsgPort *port;
    struct timerequest *request;
    struct Library *timer_base;
    int unit;
    int open;
};

/* Global timer device instances for different units */
static struct timer_device microhz_timer = {0};
static struct timer_device eclock_timer = {0};
static struct timer_device vblank_timer = {0};

/* Forward declarations */
static int _init_timer_device(struct timer_device *timer, int unit);
static void _cleanup_timer_device(struct timer_device *timer);
static int _get_timer_device(struct timer_device **timer, int unit);

/*
 * Initialize a timer device for a specific unit
 *
 * Parameters:
 *   timer: timer device structure to initialize
 *   unit: timer device unit (UNIT_MICROHZ, UNIT_ECLOCK, etc.)
 *
 * Returns: 0 on success, -1 on error with errno set
 */
static int _init_timer_device(struct timer_device *timer, int unit)
{
    /* Check for abort signal */
    __chkabort();
    
    /* Create message port */
    timer->port = CreatePort(0, 0);
    if (!timer->port) {
        errno = ENOMEM;
        return -1;
    }
    
    /* Create timer request */
    timer->request = (struct timerequest *)CreateExtIO(timer->port, sizeof(struct timerequest));
    if (!timer->request) {
        DeletePort(timer->port);
        timer->port = NULL;
        errno = ENOMEM;
        return -1;
    }
    
    /* Open timer device */
    if (OpenDevice(TIMERNAME, unit, (struct IORequest *)timer->request, 0) != 0) {
        DeleteExtIO((struct IORequest *)timer->request);
        DeletePort(timer->port);
        timer->request = NULL;
        timer->port = NULL;
        errno = ENODEV;
        return -1;
    }
    
    /* Get timer base for function calls */
    timer->timer_base = (struct Library *)timer->request->tr_node.io_Device;
    timer->unit = unit;
    timer->open = 1;
    
    return 0;
}

/*
 * Cleanup a timer device
 *
 * Parameters:
 *   timer: timer device structure to cleanup
 */
static void _cleanup_timer_device(struct timer_device *timer)
{
    if (timer->open) {
        /* Abort any pending requests */
        if (timer->request && !CheckIO((struct IORequest *)timer->request)) {
            AbortIO((struct IORequest *)timer->request);
        }
        
        /* Wait for completion */
        if (timer->request) {
            WaitIO((struct IORequest *)timer->request);
        }
        
        /* Close device */
        CloseDevice((struct IORequest *)timer->request);
        
        /* Cleanup structures */
        DeleteExtIO((struct IORequest *)timer->request);
        DeletePort(timer->port);
        
        timer->request = NULL;
        timer->port = NULL;
        timer->timer_base = NULL;
        timer->open = 0;
    }
}

/*
 * Get a timer device for a specific unit
 *
 * Parameters:
 *   timer: pointer to store timer device pointer
 *   unit: timer device unit
 *
 * Returns: 0 on success, -1 on error with errno set
 */
static int _get_timer_device(struct timer_device **timer, int unit)
{
    struct timer_device *dev;
    
    /* Select appropriate timer device */
    switch (unit) {
        case UNIT_MICROHZ:
            dev = &microhz_timer;
            break;
        case UNIT_ECLOCK:
            dev = &eclock_timer;
            break;
        case UNIT_VBLANK:
            dev = &vblank_timer;
            break;
        default:
            errno = EINVAL;
            return -1;
    }
    
    /* Initialize if not already open */
    if (!dev->open) {
        if (_init_timer_device(dev, unit) != 0) {
            return -1;
        }
    }
    
    *timer = dev;
    return 0;
}

/*
 * High-precision sleep using timer device
 *
 * Parameters:
 *   seconds: seconds to sleep
 *   microseconds: microseconds to sleep
 *   unit: timer device unit to use
 *
 * Returns: 0 on success, -1 on error with errno set
 */
int timer_device_sleep(long seconds, long microseconds, int unit)
{
    struct timer_device *timer;
    int result;
    
    /* Check for abort signal */
    __chkabort();
    
    /* Get timer device */
    if (_get_timer_device(&timer, unit) != 0) {
        return -1;
    }
    
    /* Normalize microseconds */
    if (microseconds >= 1000000) {
        seconds += microseconds / 1000000;
        microseconds = microseconds % 1000000;
    }
    
    /* Set up timer request */
    timer->request->tr_node.io_Command = TR_ADDREQUEST;
    timer->request->tr_time.tv_secs = seconds;
    timer->request->tr_time.tv_micro = microseconds;
    
    /* Submit request and wait */
    result = DoIO((struct IORequest *)timer->request);
    
    if (result != 0) {
        errno = EIO;
        return -1;
    }
    
    return 0;
}

/*
 * Get high-precision time using timer device
 *
 * Parameters:
 *   tv: timeval structure to fill
 *   unit: timer device unit to use
 *
 * Returns: 0 on success, -1 on error with errno set
 */
int timer_device_gettime(struct timeval *tv, int unit)
{
    struct timer_device *timer;
    int result;
    
    /* Check for abort signal */
    __chkabort();
    
    if (tv == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    /* Get timer device */
    if (_get_timer_device(&timer, unit) != 0) {
        return -1;
    }
    
    /* Set up timer request */
    timer->request->tr_node.io_Command = TR_GETSYSTIME;
    
    /* Submit request and wait */
    result = DoIO((struct IORequest *)timer->request);
    
    if (result != 0) {
        errno = EIO;
        return -1;
    }
    
    /* Copy result */
    *tv = timer->request->tr_time;
    
    return 0;
}

/*
 * Get E-Clock time (highest precision)
 *
 * Parameters:
 *   ec: EClockVal structure to fill
 *   freq: pointer to store E-Clock frequency
 *
 * Returns: 0 on success, -1 on error with errno set
 */
int timer_device_geteclock(struct EClockVal *ec, ULONG *freq)
{
    struct timer_device *timer;
    ULONG eclock_freq;
    
    /* Check for abort signal */
    __chkabort();
    
    if (ec == NULL || freq == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    /* Get E-Clock timer device */
    if (_get_timer_device(&timer, UNIT_ECLOCK) != 0) {
        return -1;
    }
    
    /* Read E-Clock */
    eclock_freq = ReadEClock(ec);
    *freq = eclock_freq;
    
    return 0;
}

/*
 * Cleanup all timer devices
 */
void timer_device_cleanup_all(void)
{
    _cleanup_timer_device(&microhz_timer);
    _cleanup_timer_device(&eclock_timer);
    _cleanup_timer_device(&vblank_timer);
}
