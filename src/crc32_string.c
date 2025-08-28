/*
 * SPDX-License-Identifier: BSD-2-Clause
 * crc32_string.c - Calculate CRC32 for a null-terminated string
 *
 * Copyright (C) 2025 by amigazen project
 *
 * This function provides a convenience wrapper to calculate CRC32 for strings.
 */

#include <string.h>
#include "include/stdlib.h"

/*
 * crc32_string() - Calculate CRC32 for a null-terminated string
 * 
 * Convenience function to calculate CRC32 for a string.
 *
 * Parameters:
 *   str - Null-terminated string
 *
 * Returns:
 *   CRC32 checksum for the string
 */
uint32_t crc32_string(const char *str)
{
    return crc32(str, strlen(str));
}
