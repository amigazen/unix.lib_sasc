/*
 * amiga_terminfo.h - Internal terminfo structures and functions
 *
 * This header contains internal data structures and function prototypes
 * for the Amiga terminfo implementation. Not part of the public API.
 *
 * Copyright (c) 2025 amigazen project
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef _AMIGA_TERMINFO_H
#define _AMIGA_TERMINFO_H

#include "/terminfo.h"
#include "/termcap.h"

/* Internal terminal structure */
struct terminal {
    char *term_name;           /* Terminal name (e.g., "amiga-console") */
    char *term_type;           /* Terminal type (same as term_name) */
    int columns;               /* Number of columns */
    int lines;                 /* Number of lines */
    int colors;                /* Number of colors */
    char *capabilities;        /* Raw capability string from termcap */
    struct tinfo *tinfo;       /* Enhanced termcap info structure */
    int initialized;           /* Initialization flag */
};

/* Internal function prototypes */
int amiga_terminfo_load_terminal(const char *term_name, TERMINAL **term);
int amiga_terminfo_parse_capabilities(TERMINAL *term);
void amiga_terminfo_free_terminal(TERMINAL *term);
char *amiga_terminfo_tparm_internal(const char *str, long p1, long p2, long p3, 
                                   long p4, long p5, long p6, long p7, long p8, long p9);

/* Capability name mapping from terminfo to termcap */
struct cap_mapping {
    const char *terminfo_name;
    const char *termcap_name;
    int type;  /* 0=string, 1=number, 2=flag */
};

extern const struct cap_mapping amiga_cap_mappings[];

/* Internal capability lookup */
const char *amiga_terminfo_map_capname(const char *terminfo_name);
int amiga_terminfo_get_cap_type(const char *terminfo_name);

#endif /* _AMIGA_TERMINFO_H */
