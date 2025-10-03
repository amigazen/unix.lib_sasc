/*
 * SPDX-License-Identifier: BSD-2-Clause
 * amiga_path_utils.c - Amiga-specific path conversion utilities
 *
 * This module provides utilities for converting Unix-style paths to Amiga
 * conventions, handling special device mappings, and path normalization.
 * Based on the amigaizepath.c implementation from the Amiga Perl 3 port.
 *
 * Copyright (C) 2025 by amigazen project
 */

#include "amiga.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

/*
 * amigaizepath - Convert Unix-style paths to Amiga conventions
 *
 * Special conversions:
 *   "/dev/null" -> "NULL:"
 *   "/dev/tty" -> "CON:"
 *   "/dev/console" -> "CON:"
 *   "./" -> removed (current directory)
 *   "../" -> "/" (parent directory)
 *   "." -> "" (current directory)
 *   ".." -> "/" (parent directory)
 *
 * Parameters:
 *   to - Input/output string to convert (modified in place)
 *
 * Returns:
 *   Number of characters removed during conversion
 */
int amigaizepath(char *to)
{
    char *from = to;
    int last = 0, removed = 0;

    if (!to) {
        return 0;
    }

    /* Skip leading spaces */
    while (*from == ' ') {
        from++;
        removed++;
    }

    /* Handle special device files */
    if (*from == '/') {
        if (strncmp(from, "/dev/", 5) != 0) {
            return removed;
        }

        from += 5;
        if (strcmp(from, "null") == 0) {
            strcpy(to, "NULL:");
            removed += 4;
        } else if (strcmp(from, "tty") == 0) {
            strcpy(to, "CON:");
            removed += 4;
        } else if (strcmp(from, "console") == 0) {
            strcpy(to, "CON:");
            removed += 8;
        }
        return removed;
    }

    /* Process the rest of the path */
    while (*from) {
        if (*from == '.') {
            /* Found ".", need special handling? */
            /* If beginning of line or last char was not alphanumeric */
            if (!last || !isalnum(last)) {
                if (!last) {
                    /* Look for plain "." & ".." */
                    if (!*(from + 1)) {
                        *from = '\0';  /* Change it to "" */
                        return 0;
                    } else if (*(from + 1) == '.' && !*(from + 2)) {
                        *from = '/';   /* Change it to "/" */
                        *(from + 1) = '\0';
                        return 0;
                    }
                }

                if (*(from + 1) == '/') {
                    from += 2;         /* Skip "./" */
                    removed += 2;
                    last = '/';
                    continue;          /* And go on to next char */
                } else if (*(from + 1) == '.') {
                    if (*(from + 2) == '/') {  /* Found "../"; skip ".." and */
                        from += 2;     /*  fall through allowing '/' */
                        removed += 2;
                    }
                }
            }
        }
        last = *from;
        *to++ = *from++;
    }
    *to = '\0';
    return removed;
}

/*
 * amiga_normalize_path - Normalize a path for AmigaOS
 *
 * This function performs comprehensive path normalization including:
 * - Converting Unix separators to Amiga conventions
 * - Handling relative path components
 * - Resolving special device mappings
 *
 * Parameters:
 *   path - Input path to normalize
 *   result - Output buffer for normalized path
 *   maxlen - Maximum length of result buffer
 *
 * Returns:
 *   0 on success, -1 on error (buffer too small)
 */
int amiga_normalize_path(const char *path, char *result, size_t maxlen)
{
    char *temp_path;
    size_t path_len;
    int ret;

    if (!path || !result || maxlen == 0) {
        return -1;
    }

    path_len = strlen(path);
    if (path_len >= maxlen) {
        return -1;  /* Buffer too small */
    }

    /* Allocate temporary buffer for processing */
    temp_path = malloc(path_len + 1);
    if (!temp_path) {
        return -1;
    }

    /* Copy and process the path */
    strcpy(temp_path, path);
    ret = amigaizepath(temp_path);
    
    /* Copy result if it fits */
    if (strlen(temp_path) < maxlen) {
        strcpy(result, temp_path);
        ret = 0;
    } else {
        ret = -1;  /* Result too long */
    }

    free(temp_path);
    return ret;
}

/*
 * amiga_is_device_file - Check if path refers to a special Amiga device
 *
 * Parameters:
 *   path - Path to check
 *
 * Returns:
 *   1 if path is a special device, 0 otherwise
 */
int amiga_is_device_file(const char *path)
{
    if (!path) {
        return 0;
    }

    return (strcmp(path, "NIL:") == 0 ||
            strcmp(path, "NULL:") == 0 ||
            strcmp(path, "CON:") == 0 ||
            strcmp(path, "PRT:") == 0 ||
            strcmp(path, "SER:") == 0 ||
            strcmp(path, "PAR:") == 0);
}

/*
 * amiga_get_device_mapping - Get Amiga device name for Unix device
 *
 * Parameters:
 *   unix_dev - Unix device path (e.g., "/dev/null")
 *
 * Returns:
 *   Amiga device name, or NULL if no mapping exists
 */
const char *amiga_get_device_mapping(const char *unix_dev)
{
    if (!unix_dev) {
        return NULL;
    }

    if (strcmp(unix_dev, "/dev/null") == 0) {
        return "NULL:";
    } else if (strcmp(unix_dev, "/dev/tty") == 0) {
        return "CON:";
    } else if (strcmp(unix_dev, "/dev/console") == 0) {
        return "CON:";
    } else if (strcmp(unix_dev, "/dev/printer") == 0) {
        return "PRT:";
    } else if (strcmp(unix_dev, "/dev/serial") == 0) {
        return "SER:";
    } else if (strcmp(unix_dev, "/dev/parallel") == 0) {
        return "PAR:";
    }

    return NULL;
}

/*
 * amiga_expand_tilde - Expand ~ to home directory
 *
 * Parameters:
 *   path - Path containing ~ to expand
 *   result - Output buffer for expanded path
 *   maxlen - Maximum length of result buffer
 *
 * Returns:
 *   0 on success, -1 on error
 */
int amiga_expand_tilde(const char *path, char *result, size_t maxlen)
{
    const char *home;
    size_t path_len, home_len;

    if (!path || !result || maxlen == 0) {
        return -1;
    }

    if (*path != '~') {
        /* No tilde to expand */
        if (strlen(path) >= maxlen) {
            return -1;
        }
        strcpy(result, path);
        return 0;
    }

    /* Get home directory */
    home = getenv("HOME");
    if (!home) {
        /* No HOME environment variable, leave ~ as is */
        if (strlen(path) >= maxlen) {
            return -1;
        }
        strcpy(result, path);
        return 0;
    }

    home_len = strlen(home);
    path_len = strlen(path + 1);  /* Skip the ~ */

    if (home_len + path_len >= maxlen) {
        return -1;  /* Buffer too small */
    }

    strcpy(result, home);
    if (path_len > 0) {
        strcat(result, path + 1);
    }

    return 0;
}
