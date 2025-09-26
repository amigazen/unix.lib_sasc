/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * sys/times.h - Process times
 * 
 * This header provides process timing functions for Amiga
 * 
 * Copyright (C) 2025 by amigazen project
 */

#ifndef _SYS_TIMES_H
#define _SYS_TIMES_H 1

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Clock ticks per second */
#define CLK_TCK 60

/* Process times structure */
struct tms {
    clock_t tms_utime;   /* User CPU time */
    clock_t tms_stime;   /* System CPU time */
    clock_t tms_cutime;  /* User CPU time of children */
    clock_t tms_cstime;  /* System CPU time of children */
};

/* Function prototypes */
clock_t times(struct tms *buffer);

#ifdef __cplusplus
}
#endif

#endif /* _SYS_TIMES_H */
