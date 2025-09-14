/*
 * SPDX-License-Identifier: BSD-2-Clause
 * fread.c - Read from file stream (POSIX compliant)
 *
 * This function reads data from a file stream into a buffer. It provides
 * efficient block reading with proper error handling.
 *
 * POSIX.1-2001, POSIX.1-2008, C89, C99
 * Copyright (C) 2025 by amigazen project
 */

#ifndef _WITH_SCLIB

#include "amiga.h"
#include <stdio.h>
#include <errno.h>

/* Forward declarations */
extern int fgetc(FILE *stream);

/**
 * @brief Read from file stream
 * @param ptr Pointer to buffer to store data
 * @param size Size of each element in bytes
 * @param count Number of elements to read
 * @param stream File stream to read from
 * @return Number of elements successfully read
 * 
 * The fread() function reads up to count elements of size bytes each
 * from the stream pointed to by stream, storing them at the location
 * given by ptr.
 * 
 * This implementation:
 * - Reads data element by element for reliability
 * - Handles partial reads correctly
 * - Returns number of complete elements read
 * - Maintains C89 compliance for SAS/C compiler
 * - Uses fgetc() for character-by-character reading
 */
size_t fread(void *ptr, size_t size, size_t count, FILE *stream)
{
    char *buffer;
    size_t max_bytes;
    size_t bytes_left;
    int c;
    
    /* Validate parameters */
    if (ptr == NULL || stream == NULL || size == 0 || count == 0) {
        return 0;
    }
    
    buffer = (char *)ptr;
    max_bytes = bytes_left = count * size;
    
    /* Read data character by character */
    while (bytes_left > 0) {
        c = fgetc(stream);
        if (c == EOF) {
            break;
        }
        *buffer++ = (char)c;
        bytes_left--;
    }
    
    /* Return number of complete elements read */
    return (size != 0) ? ((max_bytes - bytes_left) / size) : 0;
}

#else
/* Empty - functions provided by SAS/C sc.lib */
#endif
