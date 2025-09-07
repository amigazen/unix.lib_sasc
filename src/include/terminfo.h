/*
 * terminfo.h - POSIX terminfo interface
 *
 * This header provides the standard POSIX terminfo API functions
 * for terminal capability management. Built on top of our termcap
 * implementation for Amiga console integration.
 *
 * Copyright (c) 2025 amigazen Project
 * 
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef _TERMINFO_H
#define _TERMINFO_H

#include <sys/types.h>
#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <limits.h>
#include <ctype.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
typedef struct terminal TERMINAL;

/* Global variables */
extern TERMINAL *cur_term;        /* Current terminal */
extern char ttytype[256];         /* Terminal type name */

/* Error codes for setupterm */
#define TERMINFO_SUCCESS     0    /* Success */
#define TERMINFO_ERROR      -1    /* General error */
#define TERMINFO_BAD_TERM   -2    /* Unknown terminal type */
#define TERMINFO_NO_ACCESS  -3    /* Cannot access terminfo database */

/* Core terminfo functions */
int setupterm(const char *term, int fildes, int *errret);
TERMINAL *set_curterm(TERMINAL *nterm);
int del_curterm(TERMINAL *oterm);

/* Capability retrieval functions */
char *tigetstr(const char *capname);
int tigetnum(const char *capname);
int tigetflag(const char *capname);

/* Parameterized string functions */
char *tparm(const char *str, ...);
char *tiparm(const char *str, long p1, long p2, long p3, long p4, long p5, 
             long p6, long p7, long p8, long p9);

/* Output functions */
int tputs(const char *str, int affcnt, int (*putc)(int));
int putp(const char *str);

/* Terminal control functions */
int resetterm(void);
int fixterm(void);
int saveterm(void);

/* Utility functions */
int tigetent(char *bp, const char *name);
int tigetnum_static(const char *capname);
int tigetflag_static(const char *capname);
char *tigetstr_static(const char *capname);

#ifdef __cplusplus
}
#endif

#endif /* _TERMINFO_H */
