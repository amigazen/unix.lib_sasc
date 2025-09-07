/*
 * t_enhanced.c - Enhanced termcap API implementation for AmigaOS
 *
 * Copyright (c) 2025 amigazen project
 *
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * This file implements the enhanced t_* API functions that provide
 * better memory management and more features than the legacy API.
 * All code is written specifically for AmigaOS termcap implementation.
 */

#include "amiga_termcap_private.h"
#include "termcap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* Forward declarations */
char *t_getstr(struct tinfo *info, const char *id, char **area, size_t *limit);

/*
 * t_setinfo - Set termcap entry from arbitrary string
 * 
 * This function creates a tinfo structure from a custom termcap entry string.
 * Useful for testing or providing fallback entries.
 */
int
t_setinfo(struct tinfo **bp, const char *entry)
{
    char capability[256], *cap_ptr;
    size_t limit;
    
    if (!bp || !entry) {
        errno = EINVAL;
        return -1;
    }
    
    *bp = amiga_termcap_malloc(sizeof(struct tinfo));
    if (!*bp) {
        errno = ENOMEM;
        return -1;
    }

    (*bp)->info = amiga_termcap_malloc(strlen(entry) + 1);
    if (!(*bp)->info) {
        amiga_termcap_free(*bp);
        errno = ENOMEM;
        return -1;
    }

    strcpy((*bp)->info, entry);

    /* Cache commonly used capabilities for efficiency */
    cap_ptr = capability;
    limit = 255;
    (*bp)->up = t_getstr(*bp, "up", &cap_ptr, &limit);
    if ((*bp)->up) {
        (*bp)->up = amiga_termcap_strdup((*bp)->up);
    }
    
    cap_ptr = capability;
    limit = 255;
    (*bp)->bc = t_getstr(*bp, "bc", &cap_ptr, &limit);
    if ((*bp)->bc) {
        (*bp)->bc = amiga_termcap_strdup((*bp)->bc);
    }
    
    (*bp)->tbuf = NULL;
    
    return 0;
}

/*
 * t_getent - Get extended termcap entry for terminal name
 * 
 * This function loads a termcap entry and returns a tinfo structure
 * with the full entry (not truncated like the legacy tgetent).
 */
int
t_getent(struct tinfo **bp, const char *name)
{
    char capability[256], *cap_ptr;
    size_t limit;
    int result;
    
    if (!bp || !name) {
        errno = EINVAL;
        return -1;
    }
    
    *bp = amiga_termcap_malloc(sizeof(struct tinfo));
    if (!*bp) {
        errno = ENOMEM;
        return -1;
    }
    
    /* Load our embedded AmigaOS termcap database */
    result = amiga_termcap_load_embedded_db();
    if (result != 0) {
        amiga_termcap_free(*bp);
        return -1;
    }
    
    /* Find the entry for the requested terminal */
    if (strcmp(name, "amiga-console") == 0 || 
        strcmp(name, "amiga") == 0 || 
        strcmp(name, "amigaos-console") == 0) {
        
        /* Get the embedded AmigaOS entry */
        struct termcap_entry *entry = g_termcap_db->entries;
        if (entry) {
            (*bp)->info = amiga_termcap_strdup(entry->capabilities);
        } else {
            amiga_termcap_free(*bp);
            return -1;
        }
    } else {
        /* Terminal type not supported - AmigaOS only supports amiga-console */
        amiga_termcap_free(*bp);
        errno = ENOENT;
        return -1;
    }
    
    if (!(*bp)->info) {
        amiga_termcap_free(*bp);
        errno = ENOMEM;
        return -1;
    }

    /* Cache commonly used capabilities for efficiency */
    cap_ptr = capability;
    limit = 255;
    (*bp)->up = t_getstr(*bp, "up", &cap_ptr, &limit);
    if ((*bp)->up) {
        (*bp)->up = amiga_termcap_strdup((*bp)->up);
    }
    
    cap_ptr = capability;
    limit = 255;
    (*bp)->bc = t_getstr(*bp, "bc", &cap_ptr, &limit);
    if ((*bp)->bc) {
        (*bp)->bc = amiga_termcap_strdup((*bp)->bc);
    }
    
    (*bp)->tbuf = NULL;
        
    return 1;
}

/*
 * t_getnum - Get numeric capability value
 * 
 * Returns the numeric value of a capability like "co#80" or "li#25".
 * Returns -1 if the capability is not found.
 */
int
t_getnum(struct tinfo *info, const char *id)
{
    long num;

    if (!info || !id) {
        errno = EINVAL;
        return -1;
    }

    if (cgetnum(info->info, id, &num) == 0) {
        return (int)(num);
    } else {
        return -1;
    }
}

/*
 * t_getflag - Get boolean capability flag
 * 
 * Returns 1 if the capability is present, 0 if not found.
 * Boolean capabilities are just names without values (e.g., "am", "bs").
 */
int 
t_getflag(struct tinfo *info, const char *id)
{
    if (!info || !id) {
        errno = EINVAL;
        return 0;
    }
    
    return (cgetcap(info->info, id, (int)':') == (char *)1);
}

/*
 * t_getstr - Get string capability value
 * 
 * Returns a string capability like "cl=\033[2J" or "cm=\033[%d;%dH".
 * The string is placed in the area buffer and the area pointer is updated.
 * If area is NULL, only the length is returned in limit.
 */
char *
t_getstr(struct tinfo *info, const char *id, char **area, size_t *limit)
{
    char *s;
    int i;

    if (!info || !id) {
        errno = EINVAL;
        return NULL;
    }

    if (area == NULL) {
        /* When area is NULL, we need to get the string to calculate length */
        char *temp_area;
        s = cgetstr(info->info, id, &temp_area);
        if (s == NULL) {
            errno = ENOENT;
            if (limit != NULL) {
                *limit = 0;
            }
            return NULL;
        }
        /* Calculate length and free the temporary string */
        i = strlen(s);
        amiga_termcap_free(temp_area);
        if (limit != NULL) {
            *limit = i;
        }
        return NULL;
    } else {
        s = cgetstr(info->info, id, area);
        if (s == NULL) {
            errno = ENOENT;
            if (limit != NULL) {
                *limit = 0;
            }
            return NULL;
        }
    }
    
    /* Calculate the length of the string */
    i = strlen(s);
    
    /* Check if there's room in the area buffer */
    if (limit != NULL && (*limit < i + 1)) {
        errno = E2BIG;
        return NULL;
    }
    
    /* Update the area pointer and limit */
    *area += i + 1;
    if (limit != NULL) {
        *limit -= i;
    }

    return s;
}

/*
 * t_agetstr - Get string capability with automatic memory allocation
 * 
 * Returns a string capability allocated from an internal buffer.
 * The memory is managed by the tinfo structure and freed by t_freent().
 */
char *
t_agetstr(struct tinfo *info, const char *id)
{
    size_t new_size;
    struct tbuf *tb;
    
    if (!info || !id) {
        errno = EINVAL;
        return NULL;
    }
    
    if (t_getstr(info, id, NULL, &new_size) == NULL && new_size == 0) {
        /* String not found or empty */
        return NULL;
    }

    /* Check if we have enough space in current buffer */
    if ((tb = info->tbuf) == NULL || (tb->eptr - tb->ptr) < (new_size + 1)) {
        /* Need to allocate new buffer */
        if (new_size < 256) {
            new_size = 256;  /* Minimum buffer size */
        } else {
            new_size++;      /* Add space for null terminator */
        }

        tb = amiga_termcap_malloc(sizeof(*info->tbuf));
        if (!tb) {
            errno = ENOMEM;
            return NULL;
        }

        tb->data = amiga_termcap_malloc(new_size);
        if (!tb->data) {
            amiga_termcap_free(tb);
            errno = ENOMEM;
            return NULL;
        }

        tb->ptr = tb->data;
        tb->eptr = tb->data + new_size;

        /* Link into buffer chain */
        tb->next = info->tbuf;
        info->tbuf = tb;
    }
    
    return t_getstr(info, id, &tb->ptr, NULL);
}

/*
 * t_freent - Free tinfo structure and all associated memory
 * 
 * This function frees all memory allocated for a tinfo structure,
 * including the termcap entry, cached capabilities, and string buffers.
 */
void
t_freent(struct tinfo *info)
{
    struct tbuf *tb, *wb;

    if (!info) {
        return;
    }

    amiga_termcap_free(info->info);
    
    if (info->up) {
        amiga_termcap_free(info->up);
    }
    
    if (info->bc) {
        amiga_termcap_free(info->bc);
    }
    
    /* Free all string buffers */
    for (tb = info->tbuf; tb;) {
        wb = tb;
        tb = tb->next;
        amiga_termcap_free(wb->data);
        amiga_termcap_free(wb);
    }
    
    amiga_termcap_free(info);
}

/*
 * t_getterm - Get terminal name from termcap entry
 * 
 * Extracts the terminal name (first part before the first colon)
 * from the termcap entry.
 */
int
t_getterm(struct tinfo *info, char **area, size_t *limit)
{
    char *endp;
    size_t count;

    if (!info) {
        errno = EINVAL;
        return -1;
    }

    if ((endp = strchr(info->info, ':')) == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    count = endp - info->info + 1;
    
    if (area == NULL) {
        /* Just return the length */
        if (limit != NULL) {
            *limit = count;
        }
        return 0;
    } else {
        /* Copy the terminal name */
        if (limit != NULL && (count > *limit)) {
            errno = E2BIG;
            return -1;
        }

        strcpy(*area, info->info);
        if (limit != NULL) {
            *limit -= count;
        }
    }

    return 0;
}
