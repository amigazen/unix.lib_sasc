/*
 * SPDX-License-Identifier: BSD-2-Clause
 * amiga_wildcard.c - Enhanced wildcard expansion for AmigaOS
 *
 * This module provides advanced wildcard pattern matching and expansion
 * with support for Unix-style wildcards, character classes, and tilde expansion.
 * Based on the wildexpand.c implementation from the Amiga Perl port.
 *
 * Copyright (C) 2025 by amigazen project
 */

#include "amiga.h"
#include "amiga_path_utils.h"
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>

/* List of wildcard characters we need to process */
static const char wildlist[] = "\\~*?[";

/* Global state for wildcard expansion */
static char **wildcard_list = NULL;
static int wildcard_count = 0;
static int wildcard_max = 0;
static char *wildcard_workpath = NULL;
static char *wildcard_curp = NULL;

/*
 * Free a wildcard expansion result list
 *
 * Parameters:
 *   freelist - List of strings to free (can be NULL)
 */
void wildcard_free(char **freelist)
{
    char **cpp;

    if (!freelist) {
        return;
    }
    
    for (cpp = freelist; *cpp; cpp++) {
        free(*cpp);
    }
    free(freelist);
}

/*
 * Compare function for sorting directory entries
 */
static int wildcard_dir_compare(const void *a, const void *b)
{
    return strcmp(*(char **)a, *(char **)b);
}

/*
 * Add a directory/file to the expansion list
 *
 * Parameters:
 *   dir - Directory path
 *   file - File name
 */
static void wildcard_add_file(const char *dir, const char *file)
{
    char *full_path;
    size_t dir_len, file_len, total_len;

    if (wildcard_count + 1 >= wildcard_max) {
        /* Need room for final NULL */
        wildcard_max += 32;
        wildcard_list = realloc(wildcard_list, wildcard_max * sizeof(char *));
        if (!wildcard_list) {
            return;  /* Out of memory */
        }
    }

    dir_len = strlen(dir);
    file_len = strlen(file);
    total_len = dir_len + file_len + 2;  /* +2 for '/' and null terminator */

    full_path = malloc(total_len);
    if (!full_path) {
        return;  /* Out of memory */
    }

    strcpy(full_path, dir);
    if (file_len > 0) {
        if (dir_len > 0 && dir[dir_len - 1] != '/' && dir[dir_len - 1] != ':') {
            strcat(full_path, "/");
        }
        strcat(full_path, file);
    }

    wildcard_list[wildcard_count++] = full_path;
}

/*
 * Recursively expand wildcard patterns
 *
 * Parameters:
 *   pattern - Wildcard pattern to expand
 */
static void wildcard_recursive_expand(const char *pattern)
{
    char *next_wild;
    char *hold_curp = wildcard_curp;
    DIR *dp;
    struct dirent *d;
    char *pattern_start = (char *)pattern;

    /* Find next wildcard character */
    next_wild = strpbrk(pattern, wildlist);
    if (!next_wild) {
        *wildcard_curp = '\0';
        return;
    }

    /* Find start of current path component */
    while (next_wild > pattern && *next_wild != '/' && *next_wild != ':') {
        next_wild--;
    }
    if (*next_wild == '/' || *next_wild == ':') {
        next_wild++;
    }

    /* Copy static part of path */
    if (next_wild > pattern) {
        size_t copy_len = next_wild - pattern;
        strncpy(wildcard_curp, pattern, copy_len);
        wildcard_curp += copy_len;
    }
    *wildcard_curp = '\0';

    /* Open directory for expansion */
    dp = opendir(wildcard_workpath);
    if (!dp) {
        return;
    }

    /* Process each directory entry */
    while ((d = readdir(dp)) != NULL) {
        if (!d->d_ino) {
            continue;  /* Skip invalid entries */
        }
        
        /* Skip hidden files unless pattern starts with '.' */
        if (d->d_name[0] == '.' && *next_wild != '.') {
            continue;
        }

        /* Check if entry matches pattern */
        if (wildcard_match(d->d_name, next_wild)) {
            wildcard_add_file(wildcard_workpath, d->d_name);
        }
    }
    closedir(dp);

    wildcard_curp = hold_curp;
    *wildcard_curp = '\0';
}

/*
 * Match a string against a wildcard pattern
 *
 * Parameters:
 *   str - String to match
 *   pattern - Wildcard pattern
 *
 * Returns:
 *   1 if match, 0 if no match
 */
static int wildcard_match(const char *str, const char *pattern)
{
    char *cp, *cp2;
    const char *sp = str;
    char *hold_curp = wildcard_curp;
    int c, c2, patc, not;

    while ((patc = *pattern++) != '\0') {
        switch (patc) {
        case '?':
            if (!*str) {
                return 0;
            }
            str++;
            break;

        case '\\':
            if (*pattern++ != *str++) {
                return 0;
            }
            break;

        case '[':
            /* Character class matching */
            if (!(c = *str++)) {
                return 0;
            }
            not = (*pattern == '^');
            if (not) {
                pattern++;
            }

            if (!(patc = *pattern++)) {
                return 0;
            }
            if (patc == ']') {
                return 0;
            }

            for (;;) {
                if (patc == ']') {
                    break;
                }
                if (patc == '\\') {
                    if (!(patc = *pattern++)) {
                        return 0;
                    }
                }

                if (c != patc) {
                    if (!(c2 = *pattern)) {
                        return 0;
                    }
                    if (c2 == '\\') {
                        patc = c2;
                        continue;
                    }
                    pattern++;

                    if (c2 == '-') {
                        if (!(c2 = *pattern++)) {
                            return 0;
                        }
                        if (c2 == '\\') {
                            if (!(c2 = *pattern++)) {
                                return 0;
                            }
                        }
                    } else {
                        patc = c2;
                        continue;
                    }
                    if (c < patc || c > c2) {
                        if (!(patc = *pattern++)) {
                            return 0;
                        }
                        continue;
                    }
                }
                /* We found a match */
                if (not) {
                    return 0;
                }
                while (patc && patc != ']') {
                    patc = *pattern++;
                    if (patc == '\\') {
                        pattern++;
                    }
                }
                if (!patc) {
                    return 0;
                }
                not = 1;  /* Found match, break out */
                break;
            }
            if (not) {
                break;  /* No match and didn't want one */
            }
            return 0;  /* No match and wanted one */

        case '*':
            if (!*pattern) {
                return 1;  /* Match everything to end */
            }
            if (*pattern != '/') {
                /* Match any characters except path separators */
                for (cp = (char *)str; *cp; cp++) {
                    if (wildcard_match(cp, pattern)) {
                        return 1;
                    }
                }
                return 0;
            }
            /* Fall through to handle path separator */
            /* FALLTHROUGH */
        case '/':
            if (*str) {
                return 0;  /* String continues, no match */
            }
            cp = (char *)sp;  /* Copy final element into workpath */
            hold_curp = wildcard_curp;
            while (*cp) {
                *wildcard_curp++ = *cp++;
            }
            *wildcard_curp++ = '/';
            *wildcard_curp = '\0';

            /* Check if it's a directory and continue expansion */
            if (wildcard_is_directory(wildcard_workpath)) {
                if (!*pattern) {
                    wildcard_add_file(wildcard_workpath, "");
                } else {
                    wildcard_recursive_expand(pattern);
                }
            }

            wildcard_curp = hold_curp;
            *wildcard_curp = '\0';
            return 0;

        default:
            if (patc != *str++) {
                return 0;
            }
            break;
        }
    }
    return (*str == '\0');
}

/*
 * Check if a path is a directory
 *
 * Parameters:
 *   path - Path to check
 *
 * Returns:
 *   1 if directory, 0 otherwise
 */
static int wildcard_is_directory(const char *path)
{
    struct stat st;
    return (stat(path, &st) == 0 && S_ISDIR(st.st_mode));
}

/*
 * Expand wildcard patterns
 *
 * This function expands Unix-style wildcard patterns including:
 * - * matches any characters (except leading dots)
 * - ? matches single character (except leading dots)
 * - [abc] matches any character in set
 * - [a-z] matches character range
 * - [^abc] matches any character not in set
 * - \ escapes next character
 * - ~ expands to home directory
 *
 * Parameters:
 *   pattern - Wildcard pattern to expand
 *
 * Returns:
 *   Array of matching file paths (NULL-terminated), or NULL on error
 *   Caller must free result with wildcard_free()
 */
char **wildcard_expand(const char *pattern)
{
    char *expanded_pattern;
    char *home;
    size_t pattern_len, home_len;

    if (!pattern) {
        return NULL;
    }

    /* Initialize global state */
    wildcard_count = wildcard_max = 0;
    wildcard_list = NULL;
    wildcard_workpath = malloc(1024);
    if (!wildcard_workpath) {
        return NULL;
    }
    wildcard_curp = wildcard_workpath;

    /* Handle tilde expansion */
    if (*pattern == '~' && *(pattern + 1) == '/') {
        home = getenv("HOME");
        if (home) {
            home_len = strlen(home);
            pattern_len = strlen(pattern + 1);
            if (home_len + pattern_len < 1024) {
                strcpy(wildcard_workpath, home);
                if (wildcard_workpath[home_len - 1] != '/' && 
                    wildcard_workpath[home_len - 1] != ':') {
                    wildcard_workpath[home_len++] = '/';
                }
                strcpy(wildcard_workpath + home_len, pattern + 2);
                wildcard_curp += home_len + pattern_len;
            } else {
                free(wildcard_workpath);
                return NULL;
            }
        } else {
            strcpy(wildcard_workpath, pattern);
            wildcard_curp += strlen(pattern);
        }
    } else {
        strcpy(wildcard_workpath, pattern);
        wildcard_curp += strlen(pattern);
    }

    /* Expand the pattern */
    wildcard_recursive_expand(wildcard_workpath);

    /* Clean up work path */
    free(wildcard_workpath);

    /* Sort results */
    if (wildcard_list) {
        qsort(wildcard_list, wildcard_count, sizeof(char *), wildcard_dir_compare);
    } else {
        /* No matches found, return original pattern */
        wildcard_add_file("", pattern);
    }

    /* Add NULL terminator */
    if (wildcard_list) {
        wildcard_list[wildcard_count] = NULL;
    }

    return wildcard_list;
}

/*
 * Check if a string contains wildcard characters
 *
 * Parameters:
 *   str - String to check
 *
 * Returns:
 *   1 if contains wildcards, 0 otherwise
 */
int wildcard_has_pattern(const char *str)
{
    if (!str) {
        return 0;
    }
    return (strpbrk(str, wildlist) != NULL);
}
