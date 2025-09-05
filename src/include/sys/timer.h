/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * POSIX timer.h - POSIX.1b realtime timer definitions
 * 
 * This header provides the definitions for POSIX realtime timer functions
 * implemented using Amiga's realtime.library and timer.device.
 */

#ifndef _SYS_TIMER_H_
#define _SYS_TIMER_H_

#include <sys/types.h>
#include <time.h>
#include <signal.h>

/* Forward declarations */
struct pthread_attr_t;

/* Signal value union - define only if not already defined */
#ifndef _SIGVAL_DEFINED
#define _SIGVAL_DEFINED
union sigval {
    int sival_int;      /* Integer value */
    void *sival_ptr;    /* Pointer value */
};
#endif

/* Time specification structure */
struct timespec {
    time_t tv_sec;   /* seconds */
    long tv_nsec;    /* nanoseconds */
};

/* Timer types */
typedef int timer_t;
typedef int clockid_t;

/* Timer creation flags */
#define TIMER_ABSTIME    0x01    /* Absolute time (not relative) */

/* Clock types for clock_gettime/clock_settime */
#define CLOCK_REALTIME           0   /* System-wide realtime clock */
#define CLOCK_MONOTONIC          1   /* Monotonic clock (not affected by system time changes) */
#define CLOCK_PROCESS_CPUTIME_ID 2   /* Per-process CPU-time clock */
#define CLOCK_THREAD_CPUTIME_ID  3   /* Per-thread CPU-time clock */

/* Timer notification methods */
struct sigevent {
    int sigev_notify;              /* Notification method */
    int sigev_signo;               /* Signal number */
    union sigval sigev_value;      /* Signal value */
    void (*sigev_notify_function)(union sigval); /* Notification function */
    struct pthread_attr_t *sigev_notify_attributes;     /* Notification attributes */
};

/* Notification methods */
#define SIGEV_NONE       0       /* No notification */
#define SIGEV_SIGNAL     1       /* Generate signal */
#define SIGEV_THREAD     2       /* Call function in new thread */


/* Timer specification structure */
struct itimerspec {
    struct timespec it_interval;  /* Timer interval */
    struct timespec it_value;     /* Initial timer value */
};

/* Function prototypes */
int timer_create(clockid_t clockid, struct sigevent *sevp, timer_t *timerid);
int timer_delete(timer_t timerid);
int timer_gettime(timer_t timerid, struct itimerspec *curr_value);
int timer_settime(timer_t timerid, int flags, const struct itimerspec *new_value, struct itimerspec *old_value);

int clock_gettime(clockid_t clockid, struct timespec *tp);
int clock_settime(clockid_t clockid, const struct timespec *tp);
int clock_getres(clockid_t clockid, struct timespec *res);
int clock_nanosleep(clockid_t clockid, int flags, const struct timespec *request, struct timespec *remain);

int nanosleep(const struct timespec *request, struct timespec *remain);

#endif /* _SYS_TIMER_H_ */
