/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * monetary.h - Monetary formatting
 * 
 * This header provides monetary formatting functions for Amiga
 * 
 * Copyright (C) 2025 by amigazen project
 */

#ifndef _MONETARY_H
#define _MONETARY_H 1

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Monetary formatting flags */
#define LC_MONETARY 3   /* Monetary formatting category */

/* Function prototypes */
ssize_t strfmon(char *restrict s, size_t maxsize,
                const char *restrict format, ...);

#ifdef __cplusplus
}
#endif

#endif /* _MONETARY_H */
