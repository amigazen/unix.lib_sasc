/*
 * getppid() - Get Parent Process ID
 * 
 * SPDX-License-Identifier: BSD-2-Clause
 * Copyright (c) 2025 amigazen project
 * All rights reserved.
 */

#include <sys/types.h>
#include <unistd.h>
#include "amiga.h"
#include <proto/exec.h>

/*
 * getppid() - Get Parent Process ID
 * 
 * On AmigaOS, all processes are children of the initial process.
 * Returns 0 as the parent process ID for all processes.
 */
pid_t getppid(void)
{
    struct Process *proc;
    
    __chkabort();
    
    /* Get current process to ensure consistency with getpid() */
    proc = (struct Process *)FindTask(NULL);
    if (proc == NULL) {
        return 0; /* Fallback */
    }
    
    /* On AmigaOS, all processes are children of the initial process */
    return 0;  /* All are children of the initial process, 1 is the first user process */
}
