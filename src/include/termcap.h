/*
 * termcap.h - Terminal capability database interface
 *
 * Copyright (c) 2025 amigazen project
 *
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * This header provides the standard termcap interface for AmigaOS.
 * This is the PUBLIC API that ported software should use.
 */

#ifndef _TERMCAP_H
#define _TERMCAP_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Standard termcap functions - PUBLIC API
 * These are the functions that ported Unix software expects to find.
 */

/* Load a terminal entry from the termcap database */
int tgetent(char *buffer, char *terminal);

/* Get a string capability from the termcap entry */
char *tgetstr(char *cn, char **ptr);

/* Get a boolean capability from the termcap entry */
int tgetflag(char *id);

/* Get a numeric capability from the termcap entry */
int tgetnum(char *id);

/* Generate cursor positioning string */
char *tgoto(char *cm, int col, int line);

/* Output a string with padding */
void tputs(char *cp, int affcnt, int (*outc)(int));

/*
 * Enhanced termcap functions (BSD extensions) - PUBLIC API
 * These provide additional functionality beyond standard termcap.
 */

/* Get a terminal entry with enhanced features */
int t_getent(char *buffer, const char *terminal);

/* Get a string capability with enhanced features */
char *t_getstr(const char *cn, char **ptr);

/* Get a numeric capability with enhanced features */
int t_getnum(const char *id);

/* Get a boolean capability with enhanced features */
int t_getflag(const char *id);

/* Get a terminal entry with additional info */
int t_getterm(const char *terminal, char **termcap_entry);

/* Generate cursor positioning with enhanced features */
char *t_goto(const char *cm, int col, int line);

/* Output a string with enhanced padding */
int t_puts(const char *cp, int affcnt, int (*outc)(int));

/* Free resources associated with a termcap entry */
void t_freent(char *buffer);

/* Set additional terminal information */
int t_setinfo(const char *terminal, const char *info);

/*
 * Core capability database functions - PUBLIC API
 * These are the standard cgetcap family functions.
 */

/* Close the capability database */
int cgetclose(void);

/* Get a capability entry from the database */
int cgetent(char **buf, char **db_array, const char *name);

/* Get the first capability entry */
int cgetfirst(char **buf, char **db_array);

/* Get the next capability entry */
int cgetnext(char **buf, char **db_array);

/* Match a capability name */
int cgetmatch(const char *buf, const char *name);

/* Get a numeric capability */
int cgetnum(char *buf, const char *cap, long *num);

/* Set user-specified database */
int cgetset(const char *ent);

/* Get a string capability */
char *cgetstr(char *buf, const char *cap, char **str);

/* Get an unescaped string capability */
char *cgetustr(char *buf, const char *cap, char **str);

/* Get a capability from a buffer */
int cgetcap(char *buf, const char *cap, int type);

/* Get a boolean capability */
int cgetflag(char *buf, const char *cap);

/*
 * Global variables for compatibility
 */
extern char PC;                   /* Pad character */
extern char *BC;                  /* Backspace character */
extern char *UP;                  /* Up cursor movement */
extern short ospeed;              /* Output speed */

#ifdef __cplusplus
}
#endif

#endif /* _TERMCAP_H */