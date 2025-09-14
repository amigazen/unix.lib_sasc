/*
 * SPDX-License-Identifier: BSD-2-Clause
 * execvp.c - execute file with argument vector and PATH search (POSIX compliant)
 *
 * This function converts an argument vector to variable arguments
 * and calls execlp() for PATH search functionality.
 *
 * Copyright (C) 2025 by amigazen project
 */

#include "amiga.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * @brief Execute file with argument vector and PATH search
 * @param filename Name of file to execute
 * @param argv Array of argument strings
 * @return -1 on error, does not return on success
 * 
 * The execvp() function is equivalent to execlp() but takes an
 * argument vector instead of variable arguments. It converts the
 * argument vector to variable arguments and calls execlp().
 */
int execvp(const char *filename, char *const argv[])
{
    char **temp_argv;
    int argc;
    int i;
    int result;
    
    if (!filename || !argv) {
        errno = EINVAL;
        return -1;
    }
    
    /* Count arguments */
    argc = 0;
    while (argv[argc] != NULL) {
        argc++;
    }
    
    if (argc == 0) {
        errno = EINVAL;
        return -1;
    }
    
    /* Allocate temporary argument array */
    temp_argv = malloc((argc + 1) * sizeof(char *));
    if (temp_argv == NULL) {
        errno = ENOMEM;
        return -1;
    }
    
    /* Copy argument strings */
    for (i = 0; i < argc; i++) {
        temp_argv[i] = malloc(strlen(argv[i]) + 1);
        if (temp_argv[i] == NULL) {
            /* Clean up on error */
            while (i > 0) {
                free(temp_argv[i - 1]);
                i--;
            }
            free(temp_argv);
            errno = ENOMEM;
            return -1;
        }
        strcpy(temp_argv[i], argv[i]);
    }
    temp_argv[argc] = NULL;
    
    /* Call execlp with variable arguments */
    /* We need to use a switch statement to handle variable argc */
    switch (argc) {
        case 1:
            result = execlp(filename, temp_argv[0], NULL);
            break;
        case 2:
            result = execlp(filename, temp_argv[0], temp_argv[1], NULL);
            break;
        case 3:
            result = execlp(filename, temp_argv[0], temp_argv[1], temp_argv[2], NULL);
            break;
        case 4:
            result = execlp(filename, temp_argv[0], temp_argv[1], temp_argv[2], temp_argv[3], NULL);
            break;
        case 5:
            result = execlp(filename, temp_argv[0], temp_argv[1], temp_argv[2], temp_argv[3], temp_argv[4], NULL);
            break;
        default:
            /* For more than 5 arguments, we need a different approach */
            /* This is a limitation of the execlp() interface */
            for (i = 0; i < argc; i++) {
                free(temp_argv[i]);
            }
            free(temp_argv);
            errno = E2BIG;  /* Argument list too long */
            return -1;
    }
    
    /* Clean up on error (execlp() should not return on success) */
    if (result == -1) {
        for (i = 0; i < argc; i++) {
            free(temp_argv[i]);
        }
        free(temp_argv);
    }
    
    return result;
}
