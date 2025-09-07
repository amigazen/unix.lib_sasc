/*
 * amiga_termcap.h - Internal Amiga-specific termcap implementation
 *
 * Copyright (c) 2025 amigazen project
 *
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * This header contains INTERNAL implementation details for AmigaOS termcap.
 * These functions are used internally by the public termcap API and should
 * NOT be used directly by application code.
 */

#ifndef _INTERNAL_AMIGA_TERMCAP_H_
#define _INTERNAL_AMIGA_TERMCAP_H_

#include <exec/types.h>
#include <exec/io.h>
#include <devices/console.h>
#include <devices/input.h>
#include <devices/keymap.h>
#include <intuition/intuition.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Internal Amiga-specific termcap structure */
struct amiga_termcap {
    struct IOStdReq *con_io;      /* Console device I/O request */
    struct IOStdReq *input_io;    /* Input device I/O request */
    struct Window *window;        /* Associated intuition window */
    struct KeyMap *keymap;        /* Current keymap */
    char *termcap_entry;          /* Termcap database entry */
    char *buffer;                 /* Working buffer */
    size_t buffer_size;           /* Size of working buffer */
    int width;                    /* Terminal width */
    int height;                   /* Terminal height */
    int cursor_x;                 /* Current cursor X position */
    int cursor_y;                 /* Current cursor Y position */
    UWORD text_attr;              /* Current text attributes */
    UWORD text_fg;                /* Current foreground color */
    UWORD text_bg;                /* Current background color */
    int raw_mode;                 /* Raw input mode flag */
    int echo_mode;                /* Echo mode flag */
    int buffer_mode;              /* CON-Handler buffer mode (0=cooked, 1=raw, 2=medium) */
};

/*
 * INTERNAL FUNCTIONS - DO NOT USE DIRECTLY
 * These are implementation details used internally by the public API.
 */

/* Amiga-specific functions */
struct amiga_termcap *amiga_termcap_init(struct Window *window);
void amiga_termcap_cleanup(struct amiga_termcap *termcap);
int amiga_termcap_set_mode(struct amiga_termcap *termcap, int mode);
int amiga_termcap_get_size(struct amiga_termcap *termcap, int *width, int *height);
int amiga_termcap_set_cursor(struct amiga_termcap *termcap, int x, int y);
int amiga_termcap_clear_screen(struct amiga_termcap *termcap);
int amiga_termcap_set_attrs(struct amiga_termcap *termcap, UWORD attrs);
int amiga_termcap_get_key(struct amiga_termcap *termcap, char *key, int maxlen);

/* Amiga console device integration */
int amiga_console_open(struct amiga_termcap *termcap, struct Window *window);
void amiga_console_close(struct amiga_termcap *termcap);
int amiga_console_write(struct amiga_termcap *termcap, const char *data, int len);
int amiga_console_read(struct amiga_termcap *termcap, char *data, int len);
int amiga_console_ioctl(struct amiga_termcap *termcap, int cmd, void *arg);
int amiga_console_set_buffer_mode(struct amiga_termcap *termcap, int mode);

/* Amiga keymap integration */
int amiga_keymap_init(struct amiga_termcap *termcap);
void amiga_keymap_cleanup(struct amiga_termcap *termcap);
int amiga_keymap_convert(struct amiga_termcap *termcap, UWORD qual, UWORD code, 
                        char *buffer, int buflen);

/* Termcap database functions */
int amiga_termcap_load_db(const char *path);
int amiga_termcap_load_embedded_db(void);
void amiga_termcap_unload_db(void);
char *amiga_termcap_find_entry(const char *name);

/* Amiga-specific termcap capabilities */
#define AMIGA_CAP_CLEAR_SCREEN     "cl"    /* Clear screen */
#define AMIGA_CAP_CLEAR_EOL        "ce"    /* Clear to end of line */
#define AMIGA_CAP_CLEAR_EOS        "cd"    /* Clear to end of screen */
#define AMIGA_CAP_CURSOR_HOME      "ho"    /* Cursor home */
#define AMIGA_CAP_CURSOR_POS       "cm"    /* Cursor positioning */
#define AMIGA_CAP_CURSOR_UP        "up"    /* Cursor up */
#define AMIGA_CAP_CURSOR_DOWN      "do"    /* Cursor down */
#define AMIGA_CAP_CURSOR_LEFT      "le"    /* Cursor left */
#define AMIGA_CAP_CURSOR_RIGHT     "nd"    /* Cursor right */
#define AMIGA_CAP_BOLD_ON          "md"    /* Bold on */
#define AMIGA_CAP_BOLD_OFF         "me"    /* Bold off */
#define AMIGA_CAP_UNDERLINE_ON     "us"    /* Underline on */
#define AMIGA_CAP_UNDERLINE_OFF    "ue"    /* Underline off */
#define AMIGA_CAP_REVERSE_ON       "mr"    /* Reverse video on */
#define AMIGA_CAP_REVERSE_OFF      "me"    /* Reverse video off */
#define AMIGA_CAP_COLORS           "Co"    /* Number of colors */
#define AMIGA_CAP_COLOR_FG         "AF"    /* Set foreground color */
#define AMIGA_CAP_COLOR_BG         "AB"    /* Set background color */
#define AMIGA_CAP_TERM_WIDTH       "co"    /* Terminal width */
#define AMIGA_CAP_TERM_HEIGHT      "li"    /* Terminal height */

#ifdef __cplusplus
}
#endif

#endif /* !_INTERNAL_AMIGA_TERMCAP_H_ */
