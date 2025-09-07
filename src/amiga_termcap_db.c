/*
 * Amiga-Specific Termcap Implementation
 * Termcap database management
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "amiga_termcap_private.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global database instance */
struct termcap_db *g_termcap_db = NULL;

/*
 * amiga_termcap_load_db - load termcap database from file
 * 
 * This function loads the termcap database from a file.
 */
int
amiga_termcap_load_db(const char *path)
{
    FILE *fp;
    char line[1024];
    struct termcap_entry *entry, *last_entry;
    int line_count = 0;
    
    if (!path) {
        return AMIGA_TERMCAP_ERROR_INVALID;
    }
    
    
    /* Open database file */
    fp = fopen(path, "r");
    if (!fp) {
        return AMIGA_TERMCAP_ERROR_IO;
    }
    
    /* Allocate database structure */
    g_termcap_db = amiga_termcap_malloc(sizeof(struct termcap_db));
    if (!g_termcap_db) {
        fclose(fp);
        return AMIGA_TERMCAP_ERROR_MEMORY;
    }
    
    g_termcap_db->entries = NULL;
    g_termcap_db->entry_count = 0;
    g_termcap_db->db_path = amiga_termcap_strdup(path);
    
    last_entry = NULL;
    
    /* Read database entries */
    while (fgets(line, sizeof(line), fp)) {
        char *p, *q;
        int len;
        
        line_count++;
        
        /* Skip comments and empty lines */
        p = line;
        while (*p && (*p == ' ' || *p == '\t')) {
            p++;
        }
        if (*p == '#' || *p == '\n' || *p == '\0') {
            continue;
        }
        
        /* Remove trailing newline */
        len = amiga_termcap_strlen(p);
        if (len > 0 && p[len-1] == '\n') {
            p[len-1] = '\0';
            len--;
        }
        
        /* Skip continuation lines (starting with tab) */
        if (*p == '\t') {
            continue;
        }
        
        /* Parse entry */
        entry = amiga_termcap_malloc(sizeof(struct termcap_entry));
        if (!entry) {
            fclose(fp);
            amiga_termcap_unload_db();
            return AMIGA_TERMCAP_ERROR_MEMORY;
        }
        
        entry->name = NULL;
        entry->aliases = NULL;
        entry->capabilities = NULL;
        entry->next = NULL;
        
        /* Extract terminal name */
        q = p;
        while (*q && *q != '|' && *q != ':') {
            q++;
        }
        
        entry->name = amiga_termcap_malloc(q - p + 1);
        if (!entry->name) {
            amiga_termcap_free(entry);
            fclose(fp);
            amiga_termcap_unload_db();
            return AMIGA_TERMCAP_ERROR_MEMORY;
        }
        strncpy(entry->name, p, q - p);
        entry->name[q - p] = '\0';
        
        /* Extract aliases if present */
        if (*q == '|') {
            p = q + 1;
            q = p;
            while (*q && *q != ':') {
                q++;
            }
            
            entry->aliases = amiga_termcap_malloc(q - p + 1);
            if (!entry->aliases) {
                amiga_termcap_free(entry->name);
                amiga_termcap_free(entry);
                fclose(fp);
                amiga_termcap_unload_db();
                return AMIGA_TERMCAP_ERROR_MEMORY;
            }
            strncpy(entry->aliases, p, q - p);
            entry->aliases[q - p] = '\0';
        }
        
        /* Extract capabilities */
        if (*q == ':') {
            p = q + 1;
            entry->capabilities = amiga_termcap_strdup(p);
            if (!entry->capabilities) {
                amiga_termcap_free(entry->name);
                if (entry->aliases) {
                    amiga_termcap_free(entry->aliases);
                }
                amiga_termcap_free(entry);
                fclose(fp);
                amiga_termcap_unload_db();
                return AMIGA_TERMCAP_ERROR_MEMORY;
            }
        } else {
            entry->capabilities = amiga_termcap_strdup("");
        }
        
        /* Add to linked list */
        if (last_entry) {
            last_entry->next = entry;
        } else {
            g_termcap_db->entries = entry;
        }
        last_entry = entry;
        g_termcap_db->entry_count++;
        
    }
    
    fclose(fp);
    
    return AMIGA_TERMCAP_ERROR_NONE;
}

/*
 * amiga_termcap_unload_db - unload termcap database
 * 
 * This function unloads the termcap database and frees associated resources.
 */
void
amiga_termcap_unload_db(void)
{
    struct termcap_entry *entry, *next;
    
    
    if (!g_termcap_db) {
        return;
    }
    
    /* Free all entries */
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
    
    /* Free database structure */
    if (g_termcap_db->db_path) {
        amiga_termcap_free(g_termcap_db->db_path);
    }
    amiga_termcap_free(g_termcap_db);
    g_termcap_db = NULL;
}

/*
 * amiga_termcap_find_entry - find a termcap entry by name
 * 
 * This function searches for a termcap entry by name in the loaded database.
 */
char *
amiga_termcap_find_entry(const char *name)
{
    struct termcap_entry *entry;
    
    if (!name || !g_termcap_db) {
        return NULL;
    }
    
    
    /* Search through entries */
    entry = g_termcap_db->entries;
    while (entry) {
        /* Check main name */
        if (amiga_termcap_strcmp(entry->name, name) == 0) {
            return entry->capabilities;
        }
        
        /* Check aliases */
        if (entry->aliases) {
            char *aliases = amiga_termcap_strdup(entry->aliases);
            char *alias_start, *alias_end;
            
            alias_start = aliases;
            while (*alias_start) {
                /* Find end of current alias */
                alias_end = alias_start;
                while (*alias_end && *alias_end != '|') {
                    alias_end++;
                }
                
                /* Null-terminate current alias */
                if (*alias_end) {
                    *alias_end = '\0';
                }
                
                /* Check if this alias matches */
                if (amiga_termcap_strcmp(alias_start, name) == 0) {
                    amiga_termcap_free(aliases);
                    return entry->capabilities;
                }
                
                /* Move to next alias */
                if (*alias_end) {
                    alias_start = alias_end + 1;
                } else {
                    break;
                }
            }
            
            amiga_termcap_free(aliases);
        }
        
        entry = entry->next;
    }
    
    return NULL;
}

/*
 * amiga_termcap_parse_entry - parse a termcap entry string
 * 
 * This function parses a termcap entry string and fills in the structure.
 */
int
amiga_termcap_parse_entry(const char *entry, struct termcap_entry *parsed)
{
    char *p, *q;
    
    if (!entry || !parsed) {
        return AMIGA_TERMCAP_ERROR_INVALID;
    }
    
    /* Initialize structure */
    parsed->name = NULL;
    parsed->aliases = NULL;
    parsed->capabilities = NULL;
    parsed->next = NULL;
    
    /* Extract terminal name */
    p = (char *)entry;
    q = p;
    while (*q && *q != '|' && *q != ':') {
        q++;
    }
    
    parsed->name = amiga_termcap_malloc(q - p + 1);
    if (!parsed->name) {
        return AMIGA_TERMCAP_ERROR_MEMORY;
    }
    strncpy(parsed->name, p, q - p);
    parsed->name[q - p] = '\0';
    
    /* Extract aliases if present */
    if (*q == '|') {
        p = q + 1;
        q = p;
        while (*q && *q != ':') {
            q++;
        }
        
        parsed->aliases = amiga_termcap_malloc(q - p + 1);
        if (!parsed->aliases) {
            amiga_termcap_free(parsed->name);
            return AMIGA_TERMCAP_ERROR_MEMORY;
        }
        strncpy(parsed->aliases, p, q - p);
        parsed->aliases[q - p] = '\0';
    }
    
    /* Extract capabilities */
    if (*q == ':') {
        p = q + 1;
        parsed->capabilities = amiga_termcap_strdup(p);
        if (!parsed->capabilities) {
            amiga_termcap_free(parsed->name);
            if (parsed->aliases) {
                amiga_termcap_free(parsed->aliases);
            }
            return AMIGA_TERMCAP_ERROR_MEMORY;
        }
    } else {
        parsed->capabilities = amiga_termcap_strdup("");
    }
    
    return AMIGA_TERMCAP_ERROR_NONE;
}
