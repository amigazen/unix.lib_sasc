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
 * Note: All termcap function declarations are in unistd.h to avoid conflicts.
 * This includes:
 * - Standard termcap functions: tgetent, tgetstr, tgetnum, tgetflag, tgoto, tputs
 * - Enhanced termcap functions: t_getent, t_getstr, t_getnum, t_getflag, t_getterm, 
 *   t_goto, t_puts, t_freent, t_setinfo
 * - Core capability database functions: cgetent, cgetfirst, cgetnext, cgetmatch, 
 *   cgetnum, cgetset, cgetstr, cgetustr, cgetcap, cgetclose, cgetflag
 */

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