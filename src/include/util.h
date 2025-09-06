/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * Copyright (c) 2025 amigazen project
 *
 * POSIX utility functions header for Amiga
 */

#ifndef _UTIL_H
#define _UTIL_H

#include <sys/types.h>
#include <sys/termios.h>
#include <sys/ioctl.h>

/* Utility functions */
int login_tty(int fd);
pid_t forkpty(int *amaster, char *name, const struct termios *termp, const struct winsize *winp);
int openpty(int *amaster, int *aslave, char *name, const struct termios *termp, const struct winsize *winp);

#endif /* _UTIL_H */

