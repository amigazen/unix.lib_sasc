/*
 * getppid() - Get Parent Process ID
 * 
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2025 amigazen project
 * All rights reserved.
 */

#include <sys/types.h>
#include <unistd.h>
#include "amiga.h"

/*
 * getppid() - Get Parent Process ID
 * 
 * On AmigaOS, all processes are children of the initial process.
 * Returns 0 as the parent process ID for all processes.
 */
pid_t getppid(void)
{
    return (0);  /* All are children of the initial process, 1 is the first user process */
}
