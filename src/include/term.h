/*
 * term.h - Terminal control interface
 *
 * Copyright (c) 2025 amigazen project
 *
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * This header provides the standard terminfo/termcap interface for AmigaOS.
 * It includes both the traditional functions and the enhanced Amiga-specific
 * implementation.
 */

#ifndef _TERM_H
#define _TERM_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Standard terminfo functions
 */

/* Setup terminal */
int setupterm(const char *term, int fildes, int *errret);

/* Get terminal flag */
int tigetflag(const char *capname);

/* Get terminal number */
int tigetnum(const char *capname);

/* Get terminal string */
char *tigetstr(const char *capname);

/* Output parameterized string */
char *tparm(const char *str, ...);

/* Output string with padding */
int tputs(const char *str, int affcnt, int (*putc)(int));

/* Move cursor */
char *tgoto(const char *cm, int destcol, int destline);

/*
 * Standard termcap functions (for compatibility)
 */

/* Load terminal entry */
int tgetent(char *buffer, const char *terminal);

/* Get string capability */
char *tgetstr(const char *cn, char **ptr);

/* Get boolean capability */
int tgetflag(const char *id);

/* Get numeric capability */
int tgetnum(const char *id);

/* Generate cursor positioning string */
char *tgoto(const char *cm, int col, int line);

/* Output string with padding */
int tputs(const char *cp, int affcnt, int (*outc)(int));

/*
 * Enhanced Amiga-specific functions
 */

/* Initialize Amiga termcap system */
int amiga_termcap_init(void);

/* Get Amiga console information */
int amiga_termcap_get_console_info(struct amiga_console_info *info);

/* Set Amiga console mode */
int amiga_termcap_set_console_mode(int mode);

/* Get Amiga keymap information */
int amiga_termcap_get_keymap_info(struct amiga_keymap_info *info);

/* Set Amiga keymap */
int amiga_termcap_set_keymap(char *keymap_name);

/* Get Amiga window information */
int amiga_termcap_get_window_info(struct amiga_window_info *info);

/* Set Amiga window properties */
int amiga_termcap_set_window_properties(struct amiga_window_properties *props);

/* Cleanup Amiga termcap system */
void amiga_termcap_cleanup(void);

/*
 * Amiga-specific data structures
 */

struct amiga_console_info {
    int width;
    int height;
    int colors;
    int depth;
    int mode;
    char *device_name;
    char *window_title;
};

struct amiga_keymap_info {
    char *keymap_name;
    int keymap_type;
    int special_keys;
    int function_keys;
};

struct amiga_window_info {
    int x;
    int y;
    int width;
    int height;
    int flags;
    char *title;
};

struct amiga_window_properties {
    int x;
    int y;
    int width;
    int height;
    int flags;
    char *title;
    int close_gadget;
    int size_gadget;
    int depth_gadget;
};

/*
 * Constants
 */

/* Terminal modes */
#define AMIGA_TERMCAP_MODE_RAW     0
#define AMIGA_TERMCAP_MODE_COOKED  1
#define AMIGA_TERMCAP_MODE_MEDIUM  2

/* Window flags */
#define AMIGA_TERMCAP_WINDOW_CLOSE    0x01
#define AMIGA_TERMCAP_WINDOW_SIZE     0x02
#define AMIGA_TERMCAP_WINDOW_DEPTH    0x04
#define AMIGA_TERMCAP_WINDOW_DRAG     0x08
#define AMIGA_TERMCAP_WINDOW_BORDER   0x10

/* Keymap types */
#define AMIGA_TERMCAP_KEYMAP_US       0
#define AMIGA_TERMCAP_KEYMAP_UK       1
#define AMIGA_TERMCAP_KEYMAP_DE       2
#define AMIGA_TERMCAP_KEYMAP_FR       3
#define AMIGA_TERMCAP_KEYMAP_IT       4
#define AMIGA_TERMCAP_KEYMAP_ES       5

/* Standard terminfo return values */
#define OK      0
#define ERR     -1

/* Standard terminfo capabilities */
#define CUR_AFF 0
#define CUR_MAN 1

#ifdef __cplusplus
}
#endif

#endif /* _TERM_H */
