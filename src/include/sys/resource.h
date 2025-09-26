/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * sys/resource.h - Resource limits
 * 
 * This header provides resource limit functions for Amiga
 * 
 * Copyright (C) 2025 by amigazen project
 */

#ifndef _SYS_RESOURCE_H
#define _SYS_RESOURCE_H 1

#include <sys/time.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Resource types */
#define RLIMIT_CPU      0   /* CPU time in seconds */
#define RLIMIT_FSIZE    1   /* Maximum file size */
#define RLIMIT_DATA     2   /* Maximum data size */
#define RLIMIT_STACK    3   /* Maximum stack size */
#define RLIMIT_CORE     4   /* Maximum core file size */
#define RLIMIT_RSS      5   /* Maximum resident set size */
#define RLIMIT_MEMLOCK  6   /* Maximum locked memory */
#define RLIMIT_NPROC    7   /* Maximum number of processes */
#define RLIMIT_NOFILE   8   /* Maximum number of open files */
#define RLIMIT_SBSIZE   9   /* Maximum socket buffer size */
#define RLIMIT_VMEM     10  /* Maximum virtual memory */
#define RLIMIT_AS       11  /* Maximum address space */
#define RLIMIT_NLIMITS  12  /* Number of resource limits */

/* Resource limit structure */
struct rlimit {
    rlim_t rlim_cur;    /* Current (soft) limit */
    rlim_t rlim_max;    /* Maximum (hard) limit */
};

/* Resource usage structure */
struct rusage {
    struct timeval ru_utime;    /* User time used */
    struct timeval ru_stime;    /* System time used */
    long ru_maxrss;             /* Maximum resident set size */
    long ru_ixrss;              /* Integral shared memory size */
    long ru_idrss;              /* Integral unshared data size */
    long ru_isrss;              /* Integral unshared stack size */
    long ru_minflt;             /* Page reclaims */
    long ru_majflt;             /* Page faults */
    long ru_nswap;              /* Swaps */
    long ru_inblock;            /* Block input operations */
    long ru_oublock;            /* Block output operations */
    long ru_msgsnd;             /* Messages sent */
    long ru_msgrcv;             /* Messages received */
    long ru_nsignals;           /* Signals received */
    long ru_nvcsw;              /* Voluntary context switches */
    long ru_nivcsw;             /* Involuntary context switches */
};

/* Priority values */
#define PRIO_PROCESS    0   /* Process priority */
#define PRIO_PGRP       1   /* Process group priority */
#define PRIO_USER       2   /* User priority */

/* Function prototypes */
int getrlimit(int resource, struct rlimit *rlp);
int setrlimit(int resource, const struct rlimit *rlp);
int getrusage(int who, struct rusage *rusage);
int getpriority(int which, id_t who);
int setpriority(int which, id_t who, int value);

/* Resource limit values */
#define RLIM_INFINITY   ((rlim_t)-1)
#define RLIM_SAVED_MAX  ((rlim_t)-1)
#define RLIM_SAVED_CUR  ((rlim_t)-1)

/* Resource usage targets */
#define RUSAGE_SELF     0   /* Current process */
#define RUSAGE_CHILDREN 1   /* Child processes */

#ifdef __cplusplus
}
#endif

#endif /* _SYS_RESOURCE_H */
