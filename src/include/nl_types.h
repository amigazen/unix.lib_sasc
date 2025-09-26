/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * nl_types.h - Native language types
 * 
 * This header provides native language types for AmigaOS.
 * 
 * Copyright (C) 2025 by amigazen project
 */

#ifndef _NL_TYPES_H
#define _NL_TYPES_H 1

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Native language types */
typedef int nl_catd;
typedef int nl_item;

/* Catalog descriptor values */
#define NL_SETD 1

/* Function prototypes */
char *catgets(nl_catd catd, int set_id, int msg_id, const char *s);
char *catgetmsg(nl_catd catd, int set_id, int msg_id, const char *s);
nl_catd catopen(const char *name, int oflag);
int catclose(nl_catd catd);

/* Error return values */
#define (nl_catd)-1

#ifdef __cplusplus
}
#endif

#endif /* _NL_TYPES_H */
