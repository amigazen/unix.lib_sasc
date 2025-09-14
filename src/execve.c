/*
 * SPDX-License-Identifier: BSD-2-Clause
 * execve.c - execute file with argument vector and environment (POSIX compliant)
 *
 * This is the core exec function that all other exec family functions
 * should call. It uses the existing Amiga-specific exec() function.
 *
 * Copyright (C) 2025 by amigazen project
 */

#include "amiga.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern int exec(char *program, char **argv, int input, int output, char *dir, int stacksize);

/**
 * @brief Execute file with argument vector and environment
 * @param pathname Path to executable file
 * @param argv Array of argument strings
 * @param envp Array of environment strings
 * @return -1 on error, does not return on success
 * 
 * The execve() function replaces the current process image with a new
 * process image. This is the core exec function that all other exec
 * family functions should call.
 * 
 * On Amiga, this uses the existing exec() function with default
 * input/output and stack size.
 */
int execve(const char *pathname, char *const argv[], char *const envp[])
{
    char *path_copy;
    char **argv_copy;
    int result;
    
    if (!pathname || !argv) {
        errno = EINVAL;
        return -1;
    }
    
    /* Make a copy of the pathname since exec() may modify it */
    path_copy = malloc(strlen(pathname) + 1);
    if (!path_copy) {
        errno = ENOMEM;
        return -1;
    }
    strcpy(path_copy, pathname);
    
    /* Make a copy of argv since exec() may modify it */
    {
        int argc = 0;
        int i;
        
        /* Count arguments */
        while (argv[argc] != NULL) {
            argc++;
        }
        
        /* Allocate new argv array */
        argv_copy = malloc((argc + 1) * sizeof(char *));
        if (!argv_copy) {
            free(path_copy);
            errno = ENOMEM;
            return -1;
        }
        
        /* Copy argument strings */
        for (i = 0; i < argc; i++) {
            argv_copy[i] = malloc(strlen(argv[i]) + 1);
            if (!argv_copy[i]) {
                /* Clean up on error */
                while (i > 0) {
                    free(argv_copy[i - 1]);
                    i--;
                }
                free(argv_copy);
                free(path_copy);
                errno = ENOMEM;
                return -1;
            }
            strcpy(argv_copy[i], argv[i]);
        }
        argv_copy[argc] = NULL;
    }
    
    /* Call the Amiga-specific exec function */
    /* Use stdin/stdout and default stack size */
    result = exec(path_copy, argv_copy, 0, 1, NULL, 0);
    
    /* Clean up on error (exec() should not return on success) */
    if (result == -1) {
        int i = 0;
        while (argv_copy[i] != NULL) {
            free(argv_copy[i]);
            i++;
        }
        free(argv_copy);
        free(path_copy);
    }
    
    return result;
}
