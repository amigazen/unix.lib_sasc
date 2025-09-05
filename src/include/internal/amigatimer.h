/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * amigatimer.h - Amiga timer.device integration helper
 *
 * This header provides declarations for timer device helper functions
 * for high-precision timing operations.
 */

#ifndef AMIGATIMER_H
#define AMIGATIMER_H

#include <sys/time.h>
#include <devices/timer.h>

/* Timer device functions */
int timer_device_sleep(long seconds, long microseconds, int unit);
int timer_device_gettime(struct timeval *tv, int unit);
int timer_device_geteclock(struct EClockVal *ec, ULONG *freq);
void timer_device_cleanup_all(void);

#endif /* AMIGATIMER_H */
