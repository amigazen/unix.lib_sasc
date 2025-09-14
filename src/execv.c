/*
 * SPDX-License-Identifier: BSD-2-Clause
 * execv.c - execute file with argument vector (POSIX compliant)
 *
 * This function is a wrapper around execve() that uses the current
 * environment instead of a custom environment array.
 *
 * Copyright (C) 2025 by amigazen project
 */

#include "amiga.h"
#include <unistd.h>

extern char **environ;

/**
 * @brief Execute file with argument vector
 * @param pathname Path to executable file
 * @param argv Array of argument strings
 * @return -1 on error, does not return on success
 * 
 * The execv() function is equivalent to execve() with the current
 * environment. It calls execve() with the global environ variable.
 */
int execv(const char *pathname, char *const argv[])
{
    return execve(pathname, argv, environ);
}
