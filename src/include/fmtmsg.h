/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * fmtmsg.h - Message formatting
 * 
 * This header provides message formatting functions for Amiga
 * 
 * Copyright (C) 2025 by amigazen project
 */

#ifndef _FMTMSG_H
#define _FMTMSG_H 1

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Message classification values */
#define MM_HARD    0x01    /* Hardware error */
#define MM_SOFT    0x02    /* Software error */
#define MM_FIRM    0x04    /* Firmware error */
#define MM_APPL    0x08    /* Application error */
#define MM_UTIL    0x10    /* Utility error */
#define MM_OPSYS   0x20    /* Operating system error */

/* Message severity levels */
#define MM_NOSEV   0       /* No severity level */
#define MM_HALT    1       /* Action: Halt */
#define MM_ERROR   2       /* Action: Error */
#define MM_WARNING 3       /* Action: Warning */
#define MM_INFO    4       /* Action: Information */

/* Message source and display control */
#define MM_PRINT   0x01    /* Print to stderr */
#define MM_CONSOLE 0x02    /* Print to console */
#define MM_MSG     0x04    /* Print to message queue */
#define MM_ERR     0x08    /* Print to error stream */

/* Function prototypes */
int fmtmsg(long classification, const char *label, int severity,
           const char *text, const char *action, const char *tag);

/* Standard message labels */
#define MM_NULLLBL (char *)0
#define MM_NULLSEV 0
#define MM_NULLMC  0L
#define MM_NULLTXT (char *)0
#define MM_NULLACT (char *)0
#define MM_NULLTAG (char *)0

#ifdef __cplusplus
}
#endif

#endif /* _FMTMSG_H */
