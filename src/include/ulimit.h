/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * ulimit.h - User limits
 * 
 * This header provides user limit functions for Amiga
 * 
 * Copyright (C) 2025 by amigazen project
 */

#ifndef _ULIMIT_H
#define _ULIMIT_H 1

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Resource limit types */
#define UL_GETFSIZE 1    /* Get file size limit */
#define UL_SETFSIZE 2    /* Set file size limit */
#define UL_GMEMLIM  3    /* Get memory limit */
#define UL_SMEMLIM  4    /* Set memory limit */
#define UL_GDISLIM  5    /* Get disk limit */
#define UL_SDISLIM  6    /* Set disk limit */
#define UL_GTIMLIM  7    /* Get time limit */
#define UL_STIMLIM  8    /* Set time limit */
#define UL_GNPROC   9    /* Get process limit */
#define UL_SNPROC   10   /* Set process limit */
#define UL_GFILELIM 11   /* Get file limit */
#define UL_SFILELIM 12   /* Set file limit */
#define UL_GCORELIM 13   /* Get core limit */
#define UL_SCORELIM 14   /* Set core limit */
#define UL_GDATALIM 15   /* Get data limit */
#define UL_SDATALIM 16   /* Set data limit */
#define UL_GSTACKLIM 17  /* Get stack limit */
#define UL_SSTACKLIM 18  /* Set stack limit */

/* Function prototypes */
long ulimit(int cmd, ...);

/* Special values */
#define UL_INFINITY (-1L)

#ifdef __cplusplus
}
#endif

#endif /* _ULIMIT_H */
