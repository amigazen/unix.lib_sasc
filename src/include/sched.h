/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * Copyright (c) 2025 amigazen project
 *
 * POSIX scheduling implementation for Amiga
 */

#ifndef _SCHED_H
#define _SCHED_H

#include <exec/types.h>

/* Forward declaration for pthread_attr_t - defined in pthread.h */
typedef struct pthread_attr pthread_attr_t;

/* Scheduling policies */
#define SCHED_OTHER     0
#define SCHED_FIFO      1
#define SCHED_RR        2

/* Scheduling parameters */
struct sched_param {
    int sched_priority;
};

/* Thread scheduling functions */
int sched_yield(void);
int sched_get_priority_min(int policy);
int sched_get_priority_max(int policy);
int sched_setparam(pid_t pid, const struct sched_param *param);
int sched_setscheduler(pid_t pid, int policy, const struct sched_param *param);

/* Thread scheduling attributes */
int pthread_attr_getschedparam(const pthread_attr_t *attr, struct sched_param *param);
int pthread_attr_setschedparam(pthread_attr_t *attr, const struct sched_param *param);
int pthread_attr_getschedpolicy(const pthread_attr_t *attr, int *policy);
int pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy);
int pthread_attr_getinheritsched(const pthread_attr_t *attr, int *inherit);
int pthread_attr_setinheritsched(pthread_attr_t *attr, int inherit);
int pthread_attr_getscope(const pthread_attr_t *attr, int *scope);
int pthread_attr_setscope(pthread_attr_t *attr, int scope);


#endif /* _SCHED_H */

