/*
 * SPDX-License-Identifier: BSD-2-Clause
 * amiga.h - Amiga-specific helper functions header
 *
 * This header provides Amiga-specific helper functions that use native
 * Amiga APIs for better performance and integration.
 *
 * Copyright (C) 2025 by amigazen project
 */

#ifndef _AMIGA_H
#define _AMIGA_H 1

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Amiga-native wildcard pattern matching functions
 * 
 * These functions use native AmigaOS APIs (MatchFirst, MatchNext) for
 * efficient wildcard expansion, providing better performance than
 * generic POSIX glob() functions.
 */

/* String structure for wildcard expansion */
typedef struct {
    char *str;
} wildcard_str;

/* String list structure for managing wildcard expansion results */
typedef struct {
    wildcard_str *string;    /* Array of strings */
    int len;                 /* Current number of strings */
    int len_alloc;           /* Allocated capacity */
} wildcard_strlist;

/* Wildcard pattern matching functions */
extern void wildcard_init_list(wildcard_strlist *);
extern void wildcard_clear_list(wildcard_strlist *);
extern char *wildcard_pop_element(int, wildcard_strlist *);
extern void wildcard_push_element(char *, wildcard_strlist *);
extern void wildcard_print_list(wildcard_strlist *);
extern void wildcard_fill_list(char **, int, int, wildcard_strlist *);
extern int wildcard_scan_pattern(char *, wildcard_strlist *);
extern int wildcard_get_count(wildcard_strlist *);
extern int wildcard_is_empty(wildcard_strlist *);

/*
 * Amiga-specific macros and definitions
 */

/* CTOB - Convert C pointer to BPTR (Amiga BCPL pointer) */
#ifndef CTOB
#define CTOB(ptr) ((long)(ptr) >> 2)
#endif

/*
 * Include additional Amiga-specific utility modules
 */
#include "amiga_path_utils.h"
#include "amiga_wildcard.h"
#include "amiga_popen.h"

/*
 * Additional Amiga-specific helper functions can be added here
 * as they are implemented.
 */

#ifdef __cplusplus
}
#endif

#endif /* _AMIGA_H */
