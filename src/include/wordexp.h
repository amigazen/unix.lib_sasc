/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * wordexp.h - Word expansion
 * 
 * This header provides word expansion functions for Amiga
 * 
 * Copyright (C) 2025 by amigazen project
 */

#ifndef _WORDEXP_H
#define _WORDEXP_H 1

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Word expansion flags */
#define WRDE_APPEND     0x01    /* Append to existing words */
#define WRDE_DOOFFS     0x02    /* Use offset in we_offs */
#define WRDE_NOCMD      0x04    /* Don't perform command substitution */
#define WRDE_REUSE      0x08    /* Reuse storage from previous call */
#define WRDE_SHOWERR    0x10    /* Show errors in stderr */
#define WRDE_UNDEF      0x20    /* Error on undefined shell variables */

/* Word expansion error codes */
#define WRDE_BADCHAR    1       /* Unquoted character appears */
#define WRDE_BADVAL     2       /* Undefined shell variable */
#define WRDE_CMDSUB     3       /* Command substitution not allowed */
#define WRDE_NOSPACE    4       /* Out of memory */
#define WRDE_SYNTAX     5       /* Shell syntax error */

/* Word expansion structure */
typedef struct {
    size_t we_wordc;        /* Number of words */
    char **we_wordv;        /* Array of words */
    size_t we_offs;         /* Offset for WRDE_DOOFFS */
} wordexp_t;

/* Function prototypes */
int wordexp(const char *words, wordexp_t *pwordexp, int flags);
void wordfree(wordexp_t *pwordexp);

#ifdef __cplusplus
}
#endif

#endif /* _WORDEXP_H */
