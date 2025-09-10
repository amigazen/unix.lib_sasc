/*
 * SPDX-License-Identifier: BSD-2-Clause
 * amiga_wildcard.h - Enhanced wildcard expansion for AmigaOS header
 *
 * This header provides declarations for advanced wildcard pattern matching
 * and expansion with support for Unix-style wildcards and character classes.
 *
 * Copyright (C) 2025 by amigazen project
 */

#ifndef _AMIGA_WILDCARD_H
#define _AMIGA_WILDCARD_H 1

#ifdef __cplusplus
extern "C" {
#endif

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
char **wildcard_expand(const char *pattern);

/*
 * Free a wildcard expansion result list
 *
 * Parameters:
 *   freelist - List of strings to free (can be NULL)
 */
void wildcard_free(char **freelist);

/*
 * Check if a string contains wildcard characters
 *
 * Parameters:
 *   str - String to check
 *
 * Returns:
 *   1 if contains wildcards, 0 otherwise
 */
int wildcard_has_pattern(const char *str);

#ifdef __cplusplus
}
#endif

#endif /* _AMIGA_WILDCARD_H */
