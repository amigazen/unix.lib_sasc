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

/* Maximum number of atexit functions we can register */
#define MAX_ATEXIT_FUNCS 32

/* Static storage for atexit function pointers */
static void (*_atexit_funcs[MAX_ATEXIT_FUNCS])(void);
static int _atexit_count = 0;

/* Global cleanup functions */
void (*_fcloseall)(void);
void (*_freeall)(void);

/* Global variables */
extern short _numdev;
extern struct _device *_devtab;

/**
 * @brief Register a function to be called on program exit
 * @param func Function pointer to call on exit
 * @return 0 on success, non-zero on failure
 * 
 * The atexit() function registers a function to be called when the program
 * exits normally (via exit() or return from main()). Functions are called
 * in Last In, First Out (LIFO) order.
 */
int atexit(void (*func)(void))
{
    /* Check if we have room for another function */
    if (_atexit_count >= MAX_ATEXIT_FUNCS) {
        return -1;  /* No room for more functions */
    }
    
    /* Check for NULL function pointer */
    if (func == NULL) {
        return -1;  /* Invalid function pointer */
    }
    
    /* Register the function */
    _atexit_funcs[_atexit_count] = func;
    _atexit_count++;
    
    return 0;  /* Success */
}

/**
 * @brief Normal program termination
 * @param returnCode Exit status code
 * 
 * The exit() function causes normal program termination. The value of
 * returnCode is returned to the calling process as the program's exit status.
 * 
 * This implementation:
 * - Calls all atexit() registered functions in LIFO order
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
    int i;
    void (*atexit_func)(void);

    /* Call all atexit() registered functions in LIFO order */
    for (i = _atexit_count - 1; i >= 0; i--) {
        atexit_func = _atexit_funcs[i];
        if (atexit_func != NULL) {
            (*atexit_func)();
        }
    }

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
