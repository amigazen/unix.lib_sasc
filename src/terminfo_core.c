/*
 * terminfo_core.c - Core terminfo functions
 *
 * Implements the core POSIX terminfo API functions including
 * setupterm, capability retrieval, and terminal management.
 *
 * Copyright (c) 2025 amigazen project
 * 
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "amiga_termcap_private.h"
#include "include/internal/amiga_terminfo.h"
#include "include/terminfo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* strdup implementation for SAS/C compatibility */
char *strdup(const char *s)
{
    char *dup;
    size_t len;
    
    if (!s) return NULL;
    
    len = strlen(s) + 1;
    dup = (char *)malloc(len);
    if (dup) {
        strcpy(dup, s);
    }
    
    return dup;
}

/* Global variables */
TERMINAL *cur_term = NULL;
char ttytype[256] = "";

/* Capability name mapping from terminfo to termcap */
const struct cap_mapping amiga_cap_mappings[] = {
    /* String capabilities */
    {"clear_screen", "cl", 0},
    {"cursor_home", "ho", 0},
    {"cursor_motion", "cm", 0},
    {"cursor_down", "do", 0},
    {"cursor_left", "le", 0},
    {"cursor_right", "nd", 0},
    {"cursor_up", "up", 0},
    {"clr_eol", "ce", 0},
    {"clr_eos", "cd", 0},
    {"bell", "bl", 0},
    {"backspace", "backspace", 0},
    {"carriage_return", "cr", 0},
    {"line_feed", "do", 0},
    {"tab", "ta", 0},
    {"enter_standout_mode", "so", 0},
    {"exit_standout_mode", "se", 0},
    {"enter_underline_mode", "us", 0},
    {"exit_underline_mode", "ue", 0},
    {"enter_bold_mode", "md", 0},
    {"exit_bold_mode", "me", 0},
    {"enter_blink_mode", "mb", 0},
    {"exit_blink_mode", "me", 0},
    {"enter_dim_mode", "mh", 0},
    {"exit_dim_mode", "me", 0},
    {"enter_reverse_mode", "mr", 0},
    {"exit_reverse_mode", "me", 0},
    {"enter_secure_mode", "mk", 0},
    {"exit_secure_mode", "me", 0},
    {"enter_protected_mode", "mp", 0},
    {"exit_protected_mode", "me", 0},
    {"enter_alt_charset_mode", "as", 0},
    {"exit_alt_charset_mode", "ae", 0},
    {"enter_insert_mode", "im", 0},
    {"exit_insert_mode", "ei", 0},
    {"enter_delete_mode", "dm", 0},
    {"exit_delete_mode", "ed", 0},
    {"enter_ca_mode", "smcup", 0},
    {"exit_ca_mode", "rmcup", 0},
    {"keypad_local", "rmkx", 0},
    {"keypad_xmit", "smkx", 0},
    {"print_screen", "ps", 0},
    {"prtr_off", "pf", 0},
    {"prtr_on", "po", 0},
    {"repeat_char", "rp", 0},
    {"scroll_forward", "sf", 0},
    {"scroll_reverse", "sr", 0},
    {"set_attributes", "sa", 0},
    {"set_tab", "st", 0},
    {"set_window", "wind", 0},
    {"tab", "ht", 0},
    {"under_char", "uc", 0},
    {"up_half_line", "hu", 0},
    {"init_1string", "is1", 0},
    {"init_2string", "is2", 0},
    {"init_3string", "is3", 0},
    {"init_file", "if", 0},
    {"init_prog", "iprog", 0},
    {"key_local", "kL", 0},
    {"key_left", "kl", 0},
    {"key_right", "kr", 0},
    {"key_up", "ku", 0},
    {"key_down", "kd", 0},
    {"key_home", "kh", 0},
    {"key_ll", "kH", 0},
    {"key_sf", "kF", 0},
    {"key_sr", "kR", 0},
    {"key_npage", "kN", 0},
    {"key_ppage", "kP", 0},
    {"key_ic", "kI", 0},
    {"key_dc", "kD", 0},
    {"key_clear", "kC", 0},
    {"key_eos", "kE", 0},
    {"key_eol", "kA", 0},
    {"key_suspend", "kZ", 0},
    {"key_undo", "kU", 0},
    {"key_help", "kH", 0},
    {"key_mark", "kM", 0},
    {"key_message", "kM", 0},
    {"key_move", "kM", 0},
    {"key_next", "kN", 0},
    {"key_open", "kO", 0},
    {"key_options", "kO", 0},
    {"key_previous", "kP", 0},
    {"key_print", "kP", 0},
    {"key_redo", "kR", 0},
    {"key_reference", "kR", 0},
    {"key_refresh", "kR", 0},
    {"key_replace", "kR", 0},
    {"key_restart", "kR", 0},
    {"key_resume", "kR", 0},
    {"key_save", "kS", 0},
    {"key_sbeg", "kS", 0},
    {"key_scancel", "kS", 0},
    {"key_scommand", "kS", 0},
    {"key_scopy", "kS", 0},
    {"key_screate", "kS", 0},
    {"key_sdc", "kS", 0},
    {"key_sdl", "kS", 0},
    {"key_select", "kS", 0},
    {"key_send", "kS", 0},
    {"key_seol", "kS", 0},
    {"key_sexit", "kS", 0},
    {"key_sfind", "kS", 0},
    {"key_shelp", "kS", 0},
    {"key_shome", "kS", 0},
    {"key_sic", "kS", 0},
    {"key_sleft", "kS", 0},
    {"key_smessage", "kS", 0},
    {"key_smove", "kS", 0},
    {"key_snext", "kS", 0},
    {"key_soptions", "kS", 0},
    {"key_sprevious", "kS", 0},
    {"key_sprint", "kS", 0},
    {"key_sredo", "kS", 0},
    {"key_sreplace", "kS", 0},
    {"key_sright", "kS", 0},
    {"key_srsume", "kS", 0},
    {"key_ssave", "kS", 0},
    {"key_ssuspend", "kS", 0},
    {"key_stab", "kS", 0},
    {"key_sundo", "kS", 0},
    {"key_suspend", "kZ", 0},
    {"key_undo", "kU", 0},
    {"key_f0", "k0", 0},
    {"key_f1", "k1", 0},
    {"key_f2", "k2", 0},
    {"key_f3", "k3", 0},
    {"key_f4", "k4", 0},
    {"key_f5", "k5", 0},
    {"key_f6", "k6", 0},
    {"key_f7", "k7", 0},
    {"key_f8", "k8", 0},
    {"key_f9", "k9", 0},
    {"key_f10", "k;", 0},
    {"key_f11", "F1", 0},
    {"key_f12", "F2", 0},
    {"key_f13", "F3", 0},
    {"key_f14", "F4", 0},
    {"key_f15", "F5", 0},
    {"key_f16", "F6", 0},
    {"key_f17", "F7", 0},
    {"key_f18", "F8", 0},
    {"key_f19", "F9", 0},
    {"key_f20", "FA", 0},
    {"key_f21", "FB", 0},
    {"key_f22", "FC", 0},
    {"key_f23", "FD", 0},
    {"key_f24", "FE", 0},
    {"key_f25", "FF", 0},
    {"key_f26", "FG", 0},
    {"key_f27", "FH", 0},
    {"key_f28", "FI", 0},
    {"key_f29", "FJ", 0},
    {"key_f30", "FK", 0},
    {"key_f31", "FL", 0},
    {"key_f32", "FM", 0},
    {"key_f33", "FN", 0},
    {"key_f34", "FO", 0},
    {"key_f35", "FP", 0},
    {"key_f36", "FQ", 0},
    {"key_f37", "FR", 0},
    {"key_f38", "FS", 0},
    {"key_f39", "FT", 0},
    {"key_f40", "FU", 0},
    {"key_f41", "FV", 0},
    {"key_f42", "FW", 0},
    {"key_f43", "FX", 0},
    {"key_f44", "FY", 0},
    {"key_f45", "FZ", 0},
    {"key_f46", "Fa", 0},
    {"key_f47", "Fb", 0},
    {"key_f48", "Fc", 0},
    {"key_f49", "Fd", 0},
    {"key_f50", "Fe", 0},
    {"key_f51", "Ff", 0},
    {"key_f52", "Fg", 0},
    {"key_f53", "Fh", 0},
    {"key_f54", "Fi", 0},
    {"key_f55", "Fj", 0},
    {"key_f56", "Fk", 0},
    {"key_f57", "Fl", 0},
    {"key_f58", "Fm", 0},
    {"key_f59", "Fn", 0},
    {"key_f60", "Fo", 0},
    {"key_f61", "Fp", 0},
    {"key_f62", "Fq", 0},
    {"key_f63", "Fr", 0},
    
    /* Numeric capabilities */
    {"columns", "co", 1},
    {"lines", "li", 1},
    {"colors", "Co", 1},
    {"pairs", "pa", 1},
    {"label_height", "lh", 1},
    {"label_width", "lw", 1},
    {"max_attributes", "ma", 1},
    {"maximum_windows", "MW", 1},
    {"magic_cookie_glitch", "sg", 1},
    {"pad_char", "pc", 1},
    {"virtual_terminal", "vt", 1},
    {"num_labels", "Nl", 1},
    {"label_format", "Lf", 1},
    {"line_ins_glitch", "Lg", 1},
    {"line_del_glitch", "Ld", 1},
    {"char_ins_glitch", "Lc", 1},
    {"char_del_glitch", "Lc", 1},
    {"scroll_region", "cs", 1},
    {"new_line_glitch", "xn", 1},
    {"beep", "bl", 1},
    {"flash", "vb", 1},
    {"dummy", "dummy", 1},
    {"termcap_init", "ti", 1},
    {"termcap_reset", "te", 1},
    {"backspace_if_not_bs", "bs", 1},
    {"carriage_return_if_not_cr", "cr", 1},
    {"linefeed_if_not_lf", "do", 1},
    {"col_addr_glitch", "YA", 1},
    {"cr_cancels_micro_mode", "YB", 1},
    {"has_print_wheel", "YC", 1},
    {"row_addr_glitch", "YD", 1},
    {"semi_auto_right_margin", "YE", 1},
    {"ceol_standout_glitch", "YF", 1},
    {"eat_newline_glitch", "YG", 1},
    {"has_over_strike", "YH", 1},
    {"memory_lock", "YM", 1},
    {"memory_unlock", "YN", 1},
    {"linefeed_is_newline", "YO", 1},
    {"backspace_delay", "YP", 1},
    {"carriage_return_delay", "YQ", 1},
    {"new_line_delay", "YR", 1},
    {"over_strike_delay", "YS", 1},
    {"horizontal_tab_delay", "YT", 1},
    {"number_of_function_keys", "ku", 1},
    {"vertical_tab_delay", "YV", 1},
    {"form_feed_delay", "YW", 1},
    
    /* Boolean capabilities */
    {"auto_left_margin", "am", 2},
    {"auto_right_margin", "am", 2},
    {"no_esc_ctlc", "xsb", 2},
    {"ceol_standout_glitch", "xsb", 2},
    {"eat_newline_glitch", "xsn", 2},
    {"erase_overstrike", "eo", 2},
    {"generic_type", "gn", 2},
    {"hard_copy", "hc", 2},
    {"has_meta_key", "km", 2},
    {"has_status_line", "hs", 2},
    {"insert_null_glitch", "in", 2},
    {"memory_above", "da", 2},
    {"memory_below", "db", 2},
    {"move_insert_mode", "mi", 2},
    {"move_standout_mode", "ms", 2},
    {"over_strike", "os", 2},
    {"status_line_esc_ok", "es", 2},
    {"dest_tabs_magic_smso", "xt", 2},
    {"tilde_glitch", "hz", 2},
    {"transparent_underline", "ul", 2},
    {"xon_xoff", "xo", 2},
    {"needs_xon_xoff", "nx", 2},
    {"prtr_silent", "mc5i", 2},
    {"hard_cursor", "chts", 2},
    {"non_rev_rmcup", "nrrmc", 2},
    {"no_pad_char", "npc", 2},
    {"non_dest_scroll_region", "ndscr", 2},
    {"can_change", "ccc", 2},
    {"back_color_erase", "bce", 2},
    {"hue_lightness_saturation", "hls", 2},
    
    {NULL, NULL, 0}  /* End marker */
};

/* Internal function implementations */
const char *amiga_terminfo_map_capname(const char *terminfo_name)
{
    int i;
    
    if (!terminfo_name) return NULL;
    
    for (i = 0; amiga_cap_mappings[i].terminfo_name != NULL; i++) {
        if (strcmp(terminfo_name, amiga_cap_mappings[i].terminfo_name) == 0) {
            return amiga_cap_mappings[i].termcap_name;
        }
    }
    
    return NULL;
}

int amiga_terminfo_get_cap_type(const char *terminfo_name)
{
    int i;
    
    if (!terminfo_name) return -1;
    
    for (i = 0; amiga_cap_mappings[i].terminfo_name != NULL; i++) {
        if (strcmp(terminfo_name, amiga_cap_mappings[i].terminfo_name) == 0) {
            return amiga_cap_mappings[i].type;
        }
    }
    
    return -1;
}

int amiga_terminfo_load_terminal(const char *term_name, TERMINAL **term)
{
    TERMINAL *new_term;
    char *cap_buffer;
    int result;
    
    if (!term_name || !term) {
        errno = EINVAL;
        return -1;
    }
    
    /* Allocate terminal structure */
    new_term = (TERMINAL *)malloc(sizeof(TERMINAL));
    if (!new_term) {
        errno = ENOMEM;
        return -1;
    }
    
    /* Initialize structure */
    memset(new_term, 0, sizeof(TERMINAL));
    
    /* Allocate capability buffer - use larger size to accommodate full termcap entry */
    cap_buffer = (char *)malloc(4096);
    if (!cap_buffer) {
        free(new_term);
        errno = ENOMEM;
        return -1;
    }
    
    /* Load termcap entry */
    result = tgetent(cap_buffer, term_name);
    if (result != 1) {
        free(cap_buffer);
        free(new_term);
        return -1;
    }
    
    /* Set up terminal structure */
    new_term->term_name = strdup(term_name);
    new_term->term_type = strdup(term_name);
    new_term->capabilities = cap_buffer;
    
    /* Parse capabilities */
    if (amiga_terminfo_parse_capabilities(new_term) != 0) {
        amiga_terminfo_free_terminal(new_term);
        return -1;
    }
    
    /* Create enhanced termcap info */
    new_term->tinfo = (struct tinfo *)malloc(sizeof(struct tinfo));
    if (new_term->tinfo) {
        memset(new_term->tinfo, 0, sizeof(struct tinfo));
        new_term->tinfo->info = cap_buffer;
    }
    
    new_term->initialized = 1;
    *term = new_term;
    
    return 0;
}

int amiga_terminfo_parse_capabilities(TERMINAL *term)
{
    if (!term) return -1;
    
    /* Get basic terminal dimensions */
    term->columns = tgetnum("co");
    term->lines = tgetnum("li");
    term->colors = tgetnum("Co");
    
    /* Set defaults if not available */
    if (term->columns <= 0) term->columns = 80;
    if (term->lines <= 0) term->lines = 25;
    if (term->colors <= 0) term->colors = 16;
    
    return 0;
}

void amiga_terminfo_free_terminal(TERMINAL *term)
{
    if (!term) return;
    
    if (term->term_name) free(term->term_name);
    if (term->term_type) free(term->term_type);
    if (term->capabilities) free(term->capabilities);
    if (term->tinfo) free(term->tinfo);
    
    free(term);
}

/* Public API functions */
int setupterm(const char *term, int fildes, int *errret)
{
    char *term_name;
    int result;
    
    /* Use provided term or get from environment */
    if (term) {
        term_name = (char *)term;
    } else {
        term_name = getenv("TERM");
        if (!term_name) {
            if (errret) *errret = TERMINFO_BAD_TERM;
            return TERMINFO_ERROR;
        }
    }
    
    /* Store terminal type */
    strncpy(ttytype, term_name, sizeof(ttytype) - 1);
    ttytype[sizeof(ttytype) - 1] = '\0';
    
    /* Load terminal */
    result = amiga_terminfo_load_terminal(term_name, &cur_term);
    if (result != 0) {
        if (errret) *errret = TERMINFO_BAD_TERM;
        return TERMINFO_ERROR;
    }
    
    if (errret) *errret = TERMINFO_SUCCESS;
    return TERMINFO_SUCCESS;
}

TERMINAL *set_curterm(TERMINAL *nterm)
{
    TERMINAL *old_term = cur_term;
    cur_term = nterm;
    return old_term;
}

int del_curterm(TERMINAL *oterm)
{
    if (oterm) {
        amiga_terminfo_free_terminal(oterm);
        return TERMINFO_SUCCESS;
    }
    return TERMINFO_ERROR;
}

char *tigetstr(const char *capname)
{
    const char *termcap_name;
    char *result;
    char *area;
    
    if (!cur_term || !capname) return NULL;
    
    /* Map terminfo name to termcap name */
    termcap_name = amiga_terminfo_map_capname(capname);
    if (!termcap_name) return NULL;
    
    /* Check if it's a string capability */
    if (amiga_terminfo_get_cap_type(capname) != 0) return NULL;
    
    /* Get capability using termcap */
    area = NULL;
    result = tgetstr((char *)termcap_name, &area);
    
    return result;
}

int tigetnum(const char *capname)
{
    const char *termcap_name;
    
    if (!cur_term || !capname) return -1;
    
    /* Map terminfo name to termcap name */
    termcap_name = amiga_terminfo_map_capname(capname);
    if (!termcap_name) return -1;
    
    /* Check if it's a numeric capability */
    if (amiga_terminfo_get_cap_type(capname) != 1) return -1;
    
    /* Get capability using termcap */
    return tgetnum((char *)termcap_name);
}

int tigetflag(const char *capname)
{
    const char *termcap_name;
    int result;
    
    if (!cur_term || !capname) return -1;
    
    /* Map terminfo name to termcap name */
    termcap_name = amiga_terminfo_map_capname(capname);
    if (!termcap_name) return -1;
    
    /* Check if it's a boolean capability */
    if (amiga_terminfo_get_cap_type(capname) != 2) return -1;
    
    /* Get capability using termcap */
    result = tgetflag((char *)termcap_name);
    
    /* Return 0 for not found, 1 for found */
    return (result == 1) ? 1 : 0;
}

int resetterm(void)
{
    /* Reset terminal to initial state */
    char *area;
    char *cl_str;
    
    if (cur_term && cur_term->capabilities) {
        area = NULL;
        cl_str = tgetstr("cl", &area);
        if (cl_str) {
            printf("%s", cl_str);
            fflush(stdout);
        }
    }
    return TERMINFO_SUCCESS;
}

int fixterm(void)
{
    /* Fix terminal state */
    return TERMINFO_SUCCESS;
}

int saveterm(void)
{
    /* Save terminal state */
    return TERMINFO_SUCCESS;
}

/* Utility functions */
int tigetent(char *bp, const char *name)
{
    if (!bp || !name) return -1;
    
    /* Use tgetent to load termcap entry */
    return tgetent(bp, name);
}

int tigetnum_static(const char *capname)
{
    const char *termcap_name;
    
    if (!capname) return -1;
    
    /* Use tigetnum but don't require setupterm */
    termcap_name = amiga_terminfo_map_capname(capname);
    if (!termcap_name) return -1;
    
    /* Check if it's a numeric capability */
    if (amiga_terminfo_get_cap_type(capname) != 1) return -1;
    
    /* Get capability using termcap */
    return tgetnum(termcap_name);
}

int tigetflag_static(const char *capname)
{
    const char *termcap_name;
    
    if (!capname) return -1;
    
    /* Use tigetflag but don't require setupterm */
    termcap_name = amiga_terminfo_map_capname(capname);
    if (!termcap_name) return -1;
    
    /* Check if it's a boolean capability */
    if (amiga_terminfo_get_cap_type(capname) != 2) return -1;
    
    /* Get capability using termcap */
    return tgetflag(termcap_name);
}

char *tigetstr_static(const char *capname)
{
    const char *termcap_name;
    char *area;
    
    if (!capname) return NULL;
    
    /* Use tigetstr but don't require setupterm */
    termcap_name = amiga_terminfo_map_capname(capname);
    if (!termcap_name) return NULL;
    
    /* Check if it's a string capability */
    if (amiga_terminfo_get_cap_type(capname) != 0) return NULL;
    
    /* Get capability using termcap */
    area = NULL;
    return tgetstr(termcap_name, &area);
}
