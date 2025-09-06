/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * sched_functions.c - POSIX scheduling functions implementation
 * 
 * This file provides implementations for POSIX scheduling functions
 * that are used by posix_spawn. 
 */

#include "amiga.h"
#include <sched.h>
#include <errno.h>
#include <proto/exec.h>

/*
 * sched_setparam - Set scheduling parameters
 * 
 * On AmigaOS, this is a stub implementation that always succeeds.
 */
int sched_setparam(pid_t pid, const struct sched_param *param)
{
    /* Check for abort signal */
    __chkabort();
    
    /* Validate parameters */
    if (param == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    /* On Amiga, we can't easily set scheduling parameters */
    /* This is a stub implementation that always succeeds */
    return 0;
}

/*
 * sched_setscheduler - Set scheduling policy and parameters
 * 
 * On AmigaOS, this is a stub implementation that always succeeds.
 */
int sched_setscheduler(pid_t pid, int policy, const struct sched_param *param)
{
    /* Check for abort signal */
    __chkabort();
    
    /* Validate parameters */
    if (param == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    /* Validate policy */
    if (policy != SCHED_OTHER && policy != SCHED_FIFO && policy != SCHED_RR) {
        errno = EINVAL;
        return -1;
    }
    
    /* On Amiga, we can't easily set scheduling policy */
    /* This is a stub implementation that always succeeds */
    return 0;
}

/*
 * sched_getparam - Get scheduling parameters
 * 
 * On AmigaOS, this is a stub implementation.
 */
int sched_getparam(pid_t pid, struct sched_param *param)
{
    /* Check for abort signal */
    __chkabort();
    
    /* Validate parameters */
    if (param == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    /* Set default values */
    param->sched_priority = 0;
    
    return 0;
}

/*
 * sched_getscheduler - Get scheduling policy
 * 
 * On AmigaOS, this is a stub implementation.
 */
int sched_getscheduler(pid_t pid)
{
    /* Check for abort signal */
    __chkabort();
    
    /* Return default policy */
    return SCHED_OTHER;
}

/*
 * sched_yield - Yield the processor
 * 
 * On Amiga, this uses the SetTaskPri() trick to force rescheduling.
 */
int sched_yield(void)
{
    BYTE oldpri;
    struct Task *task;
    
    /* Check for abort signal */
    __chkabort();
    
    task = FindTask(NULL);
    if (task == NULL) {
        errno = EAGAIN;
        return -1;
    }
    
    /* Changing the priority will trigger a reschedule */
    oldpri = SetTaskPri(task, -10);
    SetTaskPri(task, oldpri);
    
    return 0;
}

/*
 * sched_get_priority_min - Get minimum priority for policy
 * 
 * Uses realistic Amiga priority range of -5 to +5.
 */
int sched_get_priority_min(int policy)
{
    /* Check for abort signal */
    __chkabort();
    
    /* Validate policy */
    if (policy != SCHED_OTHER && policy != SCHED_FIFO && policy != SCHED_RR) {
        errno = EINVAL;
        return -1;
    }
    
    /* Return minimum priority - based on pthread implementation */
    return -5;
}

/*
 * sched_get_priority_max - Get maximum priority for policy
 * 
 * Uses realistic Amiga priority range of -5 to +5.
 */
int sched_get_priority_max(int policy)
{
    /* Check for abort signal */
    __chkabort();
    
    /* Validate policy */
    if (policy != SCHED_OTHER && policy != SCHED_FIFO && policy != SCHED_RR) {
        errno = EINVAL;
        return -1;
    }
    
    /* Return maximum priority - based on pthread implementation */
    return 5;
}
