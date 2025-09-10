/*
 * SPDX-License-Identifier: BSD-2-Clause
 * amiga_path_utils.h - Amiga-specific path conversion utilities header
 *
 * This header provides declarations for utilities that convert Unix-style
 * paths to Amiga conventions and handle special device mappings.
 *
 * Copyright (C) 2025 by amigazen project
 */

#ifndef _AMIGA_PATH_UTILS_H
#define _AMIGA_PATH_UTILS_H 1

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Convert Unix-style paths to Amiga conventions
 *
 * This function modifies the input string in place, converting:
 * - "/dev/null" -> "NULL:"
 * - "/dev/tty" -> "CON:"
 * - "/dev/console" -> "CON:"
 * - "./" -> removed (current directory)
 * - "../" -> "/" (parent directory)
 * - "." -> "" (current directory)
 * - ".." -> "/" (parent directory)
 *
 * Parameters:
 *   to - Input/output string to convert (modified in place)
 *
 * Returns:
 *   Number of characters removed during conversion
 */
int amigaizepath(char *to);

/*
 * Normalize a path for AmigaOS
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
int amiga_normalize_path(const char *path, char *result, size_t maxlen);

/*
 * Check if path refers to a special Amiga device
 *
 * Parameters:
 *   path - Path to check
 *
 * Returns:
 *   1 if path is a special device, 0 otherwise
 */
int amiga_is_device_file(const char *path);

/*
 * Get Amiga device name for Unix device
 *
 * Parameters:
 *   unix_dev - Unix device path (e.g., "/dev/null")
 *
 * Returns:
 *   Amiga device name, or NULL if no mapping exists
 */
const char *amiga_get_device_mapping(const char *unix_dev);

/*
 * Expand ~ to home directory
 *
 * Parameters:
 *   path - Path containing ~ to expand
 *   result - Output buffer for expanded path
 *   maxlen - Maximum length of result buffer
 *
 * Returns:
 *   0 on success, -1 on error
 */
int amiga_expand_tilde(const char *path, char *result, size_t maxlen);

#ifdef __cplusplus
}
#endif

#endif /* _AMIGA_PATH_UTILS_H */
