/*
 * SPDX-License-Identifier: BSD-2-Clause
 * fwrite.c - Write to file stream (POSIX compliant)
 *
 * This function writes data from a buffer to a file stream. It provides
 * efficient block writing with proper error handling.
 *
 * POSIX.1-2001, POSIX.1-2008, C89, C99
 * Copyright (C) 2025 by amigazen project
 */

#ifndef _WITH_SCLIB

#include "amiga.h"
#include <stdio.h>
#include <errno.h>

/* Forward declarations */
extern int fputc(int c, FILE *stream);

/**
 * @brief Write to file stream
 * @param ptr Pointer to buffer containing data
 * @param size Size of each element in bytes
 * @param count Number of elements to write
 * @param stream File stream to write to
 * @return Number of elements successfully written
 * 
 * The fwrite() function writes up to count elements of size bytes each
 * from the array pointed to by ptr to the stream pointed to by stream.
 * 
 * This implementation:
 * - Writes data element by element for reliability
 * - Handles partial writes correctly
 * - Returns number of complete elements written
 * - Maintains C89 compliance for SAS/C compiler
 * - Uses fputc() for character-by-character writing
 */
size_t fwrite(const void *ptr, size_t size, size_t count, FILE *stream)
{
    const char *buffer;
    size_t max_bytes;
    size_t bytes_left;
    int result;
    
    /* Validate parameters */
    if (ptr == NULL || stream == NULL || size == 0 || count == 0) {
        return 0;
    }
    
    buffer = (const char *)ptr;
    max_bytes = bytes_left = count * size;
    
    /* Write data character by character */
    while (bytes_left > 0) {
        result = fputc(*buffer++, stream);
        if (result == EOF) {
            break;
        }
        bytes_left--;
    }
    
    /* Return number of complete elements written */
    return (size != 0) ? ((max_bytes - bytes_left) / size) : 0;
}

#else
/* Empty - functions provided by SAS/C sc.lib */
#endif
