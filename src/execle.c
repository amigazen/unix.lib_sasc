/*
 * SPDX-License-Identifier: BSD-2-Clause
 * execle.c - execute file with variable arguments and environment (POSIX compliant)
 *
 * This function converts variable arguments to an argument vector
 * and calls execve() with a custom environment.
 *
 * Copyright (C) 2025 by amigazen project
 */

#include "amiga.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * @brief Execute file with variable arguments and environment
 * @param pathname Path to executable file
 * @param arg0 First argument (program name)
 * @param ... Additional arguments, terminated by NULL, then environment array
 * @return -1 on error, does not return on success
 * 
 * The execle() function is equivalent to execve() but takes variable
 * arguments instead of an argument vector. The last argument must be
 * a NULL-terminated array of environment strings.
 */
int execle(const char *pathname, const char *arg0, ...)
{
    char **argv;
    char **envp;
    va_list args;
    int argc;
    int i;
    char *arg;
    
    if (!pathname || !arg0) {
        errno = EINVAL;
        return -1;
    }
    
    /* Count arguments to determine array size needed */
    va_start(args, arg0);
    argc = 1; /* arg0 */
    while ((arg = va_arg(args, char *)) != NULL) {
        argc++;
    }
    va_end(args);
    
    /* Allocate argument array */
    argv = malloc((argc + 1) * sizeof(char *));
    if (argv == NULL) {
        errno = ENOMEM;
        return -1;
    }
    
    /* Fill argument array */
    va_start(args, arg0);
    argv[0] = (char *)arg0;
    for (i = 1; i < argc; i++) {
        argv[i] = va_arg(args, char *);
    }
    argv[argc] = NULL;
    
    /* Get environment array (last argument) */
    envp = va_arg(args, char **);
    va_end(args);
    
    /* Call execve with the argument array and environment */
    return execve(pathname, argv, envp);
}
