/*
 * Amiga-Specific Termcap Implementation
 * Termcap database entry retrieval functions
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "amiga_termcap_private.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global termcap database - defined in amiga_termcap_db.c */
extern struct termcap_db *g_termcap_db;
static char *g_termcap_entry = NULL;

/*
 * cgetent - get a termcap entry from the database
 * 
 * This function searches for a termcap entry by name in the database.
 * It handles the termcap database format and returns the entry string.
 */
int
cgetent(char **buf, char **db_array, const char *name)
{
    struct termcap_entry *entry;
    char *result;
    int len;
    
    if (!buf || !name) {
        return -1;
    }
    
    
    /* Load database if not already loaded */
    if (!g_termcap_db) {
        if (amiga_termcap_load_embedded_db() != 0) {
            return -1;
        }
    }
    
    /* Search for the entry */
    entry = g_termcap_db->entries;
    while (entry) {
        if (amiga_termcap_strcmp(entry->name, name) == 0) {
            /* Found exact match */
            break;
        }
        
        /* Check aliases */
        if (entry->aliases) {
            char *alias_start, *alias_end;
            char *aliases = amiga_termcap_strdup(entry->aliases);
            
            alias_start = aliases;
            while (*alias_start) {
                /* Find end of current alias */
                alias_end = alias_start;
                while (*alias_end && *alias_end != ',') {
                    alias_end++;
                }
                
                /* Null-terminate current alias */
                if (*alias_end) {
                    *alias_end = '\0';
                }
                
                /* Check if this alias matches */
                if (amiga_termcap_strcmp(alias_start, name) == 0) {
                    amiga_termcap_free(aliases);
                    break;
                }
                
                /* Move to next alias */
                if (*alias_end) {
                    alias_start = alias_end + 1;
                } else {
                    break;
                }
            }
            
            amiga_termcap_free(aliases);
            if (alias_start && *alias_start) {
                break; /* Found alias match */
            }
        }
        
        entry = entry->next;
    }
    
    if (!entry) {
        return -1;
    }
    
    /* Allocate space for result */
    len = amiga_termcap_strlen(entry->capabilities);
    result = amiga_termcap_malloc(len + 1);
    if (!result) {
        return -1;
    }
    
    /* Copy the capabilities string */
    strcpy(result, entry->capabilities);
    *buf = result;
    
    return 0;
}

/*
 * cgetfirst - get the first termcap entry from the database
 * 
 * This function returns the first entry in the termcap database.
 */
int
cgetfirst(char **buf, char **db_array)
{
    if (!buf || !db_array) {
        return -1;
    }
    
    /* Load database if not already loaded */
    if (!g_termcap_db) {
        if (amiga_termcap_load_embedded_db() != 0) {
            return -1;
        }
    }
    
    if (!g_termcap_db->entries) {
        return -1;
    }
    
    /* Return first entry */
    return cgetent(buf, db_array, g_termcap_db->entries->name);
}

/*
 * cgetnext - get the next termcap entry from the database
 * 
 * This function returns the next entry in the termcap database.
 * It maintains state between calls to iterate through entries.
 */
int
cgetnext(char **buf, char **db_array)
{
    static struct termcap_entry *current_entry = NULL;
    
    if (!buf || !db_array) {
        return -1;
    }
    
    /* Load database if not already loaded */
    if (!g_termcap_db) {
        if (amiga_termcap_load_embedded_db() != 0) {
            return -1;
        }
    }
    
    /* Initialize current entry if needed */
    if (!current_entry) {
        current_entry = g_termcap_db->entries;
    } else {
        current_entry = current_entry->next;
    }
    
    if (!current_entry) {
        return -1;
    }
    
    /* Return current entry */
    return cgetent(buf, db_array, current_entry->name);
}

/*
 * cgetmatch - check if a termcap entry matches a name
 * 
 * This function checks if a termcap entry matches a given name,
 * including checking aliases.
 */
int
cgetmatch(const char *buf, const char *name)
{
    char *entry_name, *aliases;
    char *p, *q;
    
    if (!buf || !name) {
        return 0;
    }
    
    /* Extract terminal name from entry */
    p = (char *)buf;
    while (*p && *p != '|' && *p != ':') {
        p++;
    }
    
    if (*p == '|') {
        /* Has aliases */
        entry_name = amiga_termcap_malloc(p - buf + 1);
        if (!entry_name) {
            return 0;
        }
        strncpy(entry_name, buf, p - buf);
        entry_name[p - buf] = '\0';
        
        /* Check main name */
        if (amiga_termcap_strcmp(entry_name, name) == 0) {
            amiga_termcap_free(entry_name);
            return 1;
        }
        
        /* Check aliases */
        p++; /* Skip the | */
        q = p;
        while (*q && *q != ':') {
            q++;
        }
        
        aliases = amiga_termcap_malloc(q - p + 1);
        if (!aliases) {
            amiga_termcap_free(entry_name);
            return 0;
        }
        strncpy(aliases, p, q - p);
        aliases[q - p] = '\0';
        
        /* Check each alias */
        p = aliases;
        while (*p) {
            q = p;
            while (*q && *q != '|') {
                q++;
            }
            
            if (*q) {
                *q = '\0';
            }
            
            if (amiga_termcap_strcmp(p, name) == 0) {
                amiga_termcap_free(entry_name);
                amiga_termcap_free(aliases);
                return 1;
            }
            
            if (*q) {
                p = q + 1;
            } else {
                break;
            }
        }
        
        amiga_termcap_free(entry_name);
        amiga_termcap_free(aliases);
    } else {
        /* No aliases, just check main name */
        entry_name = amiga_termcap_malloc(p - buf + 1);
        if (!entry_name) {
            return 0;
        }
        strncpy(entry_name, buf, p - buf);
        entry_name[p - buf] = '\0';
        
        if (amiga_termcap_strcmp(entry_name, name) == 0) {
            amiga_termcap_free(entry_name);
            return 1;
        }
        
        amiga_termcap_free(entry_name);
    }
    
    return 0;
}

/*
 * cgetset - set a termcap entry from a string
 * 
 * This function sets a termcap entry from a provided string,
 * useful for programmatically creating termcap entries.
 */
int
cgetset(const char *entry)
{
    if (!entry) {
        return -1;
    }
    
    /* Free existing entry */
    if (g_termcap_entry) {
        amiga_termcap_free(g_termcap_entry);
    }
    
    /* Allocate and copy new entry */
    g_termcap_entry = amiga_termcap_strdup(entry);
    if (!g_termcap_entry) {
        return -1;
    }
    
    return 0;
}

/*
 * cgetclose - close the termcap database
 * 
 * This function closes the termcap database and frees associated resources.
 */
int
cgetclose(void)
{
    struct termcap_entry *entry, *next;
    
    
    /* Free database entries */
    if (g_termcap_db) {
        entry = g_termcap_db->entries;
        while (entry) {
            next = entry->next;
            if (entry->name) {
                amiga_termcap_free(entry->name);
            }
            if (entry->aliases) {
                amiga_termcap_free(entry->aliases);
            }
            if (entry->capabilities) {
                amiga_termcap_free(entry->capabilities);
            }
            amiga_termcap_free(entry);
            entry = next;
        }
        
        if (g_termcap_db->db_path) {
            amiga_termcap_free(g_termcap_db->db_path);
        }
        amiga_termcap_free(g_termcap_db);
        g_termcap_db = NULL;
    }
    
    /* Free global entry */
    if (g_termcap_entry) {
        amiga_termcap_free(g_termcap_entry);
        g_termcap_entry = NULL;
    }
    
    return 0;
}
