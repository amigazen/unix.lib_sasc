/*
 * SPDX-License-Identifier: BSD-2-Clause
 * fdopen.c - File descriptor to FILE* conversion
 *
 * Based on PDC fdopen.c by J.A. Lydiatt
 * Copyright (C) 2025 by amigazen project
 */

#ifndef _WITH_SCLIB

#include <stdio.h>
#include <unistd.h>
#include "amiga.h"

/* Forward declarations */
extern FILE *addStream(void);

/**
 * @brief Convert file descriptor to FILE* stream
 * @param fd File descriptor to convert
 * @param mode Mode string ("r", "w", "a", etc.)
 * @return FILE* stream or NULL on error
 * 
 * Converts a file descriptor to a buffered FILE* stream.
 * This is essential for POSIX compatibility.
 */
FILE *fdopen(int fd, const char *mode)
{
    FILE *fp;

    /* Validate file descriptor */
    if (fd < 0) {
        return NULL;
    }

    /* Get a new FILE structure */
    if ((fp = addStream()) == NULL) {
        return NULL;
    }

    /* Set file descriptor */
    fp->_file = fd;

    /* Parse mode string and set appropriate flags */
    switch (*mode) {
    case 'r':
        fp->_flag = _IOREAD;
        break;
    case 'w':
        fp->_flag = _IOWRT;
        break;
    case 'a':
        fp->_flag = _IOWRT | _IOAPP;  /* Use _IOAPP instead of _IOAPPEND */
        break;
    default:
        /* Invalid mode */
        return NULL;
    }

    /* Check for update mode (r+, w+, a+) */
    if (mode[1] == '+' || mode[2] == '+') {
        fp->_flag |= _IORW;
    }

    /* Check if it's a TTY for line buffering (binary mode doesn't exist in SAS/C) */
    if (isatty(fd)) {
        fp->_flag |= _IOLBF;
    }

    /* Initialize buffer pointers */
    fp->_ptr = fp->_base;
    fp->_rcnt = 0;
    fp->_wcnt = 0;
    fp->_size = 0;
    fp->_cbuff = 0;

    return fp;
}

#else
/* Empty - functions provided by SAS/C sc.lib */
#endif
