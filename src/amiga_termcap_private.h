/*
 * Amiga-Specific Termcap Implementation - Private Header
 * Internal structures and definitions
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef _AMIGA_TERMCAP_PRIVATE_H_
#define _AMIGA_TERMCAP_PRIVATE_H_

#include "internal/amiga_termcap.h"
#include "termcap.h"

/* Internal buffer management */
#define AMIGA_TERMCAP_BUFFER_SIZE  4096
#define AMIGA_TERMCAP_MAX_ENTRIES  256

/* Termcap database entry structure */
struct termcap_entry {
    char *name;                   /* Terminal name */
    char *aliases;                /* Aliases (comma-separated) */
    char *capabilities;           /* Capability string */
    struct termcap_entry *next;   /* Linked list pointer */
};

/* Internal termcap database */
struct termcap_db {
    struct termcap_entry *entries;    /* Linked list of entries */
    int entry_count;                  /* Number of entries */
    char *db_path;                    /* Path to database file */
};

/* Extended termcap info structure */
struct tinfo {
    char *info;                   /* Termcap entry string */
    char *up;                     /* Cursor up capability */
    char *bc;                     /* Backspace capability */
    struct amiga_termcap *amiga;  /* Amiga-specific data */
    struct tbuf {
        struct tbuf *next;
        char *data;
        char *ptr;
        char *eptr;
    } *tbuf;
};

/* Global database instance */
extern struct termcap_db *g_termcap_db;

/* Internal function prototypes */
int amiga_termcap_parse_entry(const char *entry, struct termcap_entry *parsed);
int amiga_termcap_build_capabilities(struct amiga_termcap *termcap);
int amiga_termcap_apply_capability(struct amiga_termcap *termcap, 
                                   const char *cap, const char *value);
char *amiga_termcap_expand_string(const char *str, int *len);
int amiga_termcap_handle_escape(const char **str, char *output, int maxlen);

/* Console device command helpers */
int amiga_console_send_cmd(struct amiga_termcap *termcap, UWORD cmd, 
                           APTR data, ULONG length);
int amiga_console_get_attrs(struct amiga_termcap *termcap, struct ConsoleAttribute *attrs);
int amiga_console_set_attrs(struct amiga_termcap *termcap, struct ConsoleAttribute *attrs);

/* Keymap helpers */
int amiga_keymap_get_default(struct amiga_termcap *termcap);
int amiga_keymap_set_current(struct amiga_termcap *termcap, struct KeyMap *keymap);
int amiga_keymap_qual_to_string(UWORD qual, char *buffer, int buflen);

/* String processing helpers */
int amiga_termcap_strlen(const char *str);
char *amiga_termcap_strdup(const char *str);
int amiga_termcap_strcmp(const char *s1, const char *s2);
char *amiga_termcap_strchr(const char *str, int c);
char *amiga_termcap_strstr(const char *haystack, const char *needle);

/* Memory management */
void *amiga_termcap_malloc(size_t size);
void amiga_termcap_free(void *ptr);
void *amiga_termcap_realloc(void *ptr, size_t size);

/* Error handling */
#define AMIGA_TERMCAP_ERROR_NONE       0
#define AMIGA_TERMCAP_ERROR_MEMORY     -1
#define AMIGA_TERMCAP_ERROR_DEVICE     -2
#define AMIGA_TERMCAP_ERROR_NOTFOUND   -3
#define AMIGA_TERMCAP_ERROR_INVALID    -4
#define AMIGA_TERMCAP_ERROR_IO         -5

int amiga_termcap_set_error(int error);
int amiga_termcap_get_error(void);

#endif /* !_AMIGA_TERMCAP_PRIVATE_H_ */
