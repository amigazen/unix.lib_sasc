/*
 * SPDX-License-Identifier: BSD-2-Clause
 * amiga_popen.h - Enhanced process execution for AmigaOS header
 *
 * This header provides declarations for sophisticated popen/pclose functionality
 * with proper I/O redirection parsing and AmigaOS Execute() integration.
 *
 * Copyright (C) 2025 by amigazen project
 */

#ifndef _AMIGA_POPEN_H
#define _AMIGA_POPEN_H 1

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Enhanced popen implementation for AmigaOS
 *
 * This function provides sophisticated process execution with:
 * - I/O redirection parsing
 * - Pipe handling for inter-process communication
 * - Proper AmigaOS Execute() integration
 * - Error handling and validation
 *
 * Parameters:
 *   cmd - Command string to execute
 *   mode - Open mode ("r" for read, "w" for write)
 *
 * Returns:
 *   FILE pointer on success, NULL on error
 */
FILE *amiga_popen(const char *cmd, const char *mode);

/*
 * Enhanced pclose implementation for AmigaOS
 *
 * Parameters:
 *   stream - FILE stream to close
 *
 * Returns:
 *   0 on success, -1 on error
 */
int amiga_pclose(FILE *stream);

/*
 * Check if a command exists in the system
 *
 * Parameters:
 *   cmd - Command name to check
 *
 * Returns:
 *   1 if command exists, 0 otherwise
 */
int amiga_cmd_exists_public(const char *cmd);

#ifdef __cplusplus
}
#endif

#endif /* _AMIGA_POPEN_H */
