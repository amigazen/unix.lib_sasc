/*
 * SPDX-License-Identifier: BSD-2-Clause
 * exit.c - Normal program termination
 *
 * Based on PDC exit.c by J.A. Lydiatt
 * Copyright (C) 2025 by amigazen project
 */

#ifndef _WITH_SCLIB

#include <stdlib.h>
#include <fcntl.h>

extern void _exit(int);
extern int close(int);
extern void free(void *);

/* Global cleanup functions */
void (*_fcloseall)(void);
void (*_freeall)(void);

/* Global variables */
extern short _numdev;
extern struct _device *_devtab;

/**
 * @brief Normal program termination
 * @param returnCode Exit status code
 * 
 * The exit() function causes normal program termination. The value of
 * returnCode is returned to the calling process as the program's exit status.
 * 
 * This implementation:
 * - Closes all open files if _fcloseall is set
 * - Closes all file descriptors in device table
 * - Frees the device table memory
 * - Frees all allocated memory if _freeall is set
 * - Calls _exit() with the return code
 * - Is C89 compliant for SAS/C compiler compatibility
 */
void exit(int returnCode)
{
    int fd;

    if (_fcloseall != NULL) {
        (*_fcloseall)();
    }

    if (_devtab != NULL) {
        for (fd = 0; fd < _numdev; ++fd) {
            close(fd);
        }
        free(_devtab);
    }

    if (_freeall != NULL) {
        (*_freeall)(); /* Free any malloc()ed memory */
    }

    _exit(returnCode);
}

#else
/* Empty - function provided by SAS/C sc.lib */
#endif
