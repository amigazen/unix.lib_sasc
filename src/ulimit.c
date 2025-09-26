/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * ulimit.c - get and set user limits (POSIX compliant)
 *
 * The ulimit() function provides control over process limits. This is a
 * legacy interface that predates the more comprehensive getrlimit/setrlimit
 * functions.
 *
 * POSIX.1-2001, POSIX.1-2008, SVr4, 4.3BSD
 */

#include "amiga.h"
#include <ulimit.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <stdarg.h>

/* Default limits for AmigaOS - realistic for typical Amiga hardware */
#define DEFAULT_FILE_SIZE_LIMIT    (2L * 1024L * 1024L * 1024L)  /* 2GB - 64-bit filesystem support */
#define DEFAULT_MEMORY_LIMIT       (2L * 1024L * 1024L)          /* 2MB - typical Amiga memory */
#define DEFAULT_DISK_LIMIT         (10L * 1024L * 1024L)         /* 10MB - reasonable disk usage */
#define DEFAULT_TIME_LIMIT         UL_INFINITY                   /* No time limit - AmigaOS doesn't need it */
#define DEFAULT_PROCESS_LIMIT      256L                          /* Many processes supported */
#define DEFAULT_FILE_LIMIT         256L                          /* FastFileSystem folder limit */
#define DEFAULT_CORE_LIMIT         (1L * 1024L)                  /* 1KB - Amiga doesn't use core dumps */
#define DEFAULT_DATA_LIMIT         (1L * 1024L * 1024L)          /* 1MB - data segment size */
#define DEFAULT_STACK_LIMIT        (64L * 1024L)                 /* 64KB - typical Amiga stack size */

/* Current limits (simplified - not persistent across processes) */
static long current_limits[18] = {
    0,                          /* UL_GETFSIZE - will be set on first call */
    DEFAULT_FILE_SIZE_LIMIT,    /* UL_SETFSIZE */
    DEFAULT_MEMORY_LIMIT,       /* UL_GMEMLIM */
    DEFAULT_MEMORY_LIMIT,       /* UL_SMEMLIM */
    DEFAULT_DISK_LIMIT,         /* UL_GDISLIM */
    DEFAULT_DISK_LIMIT,         /* UL_SDISLIM */
    DEFAULT_TIME_LIMIT,         /* UL_GTIMLIM */
    DEFAULT_TIME_LIMIT,         /* UL_STIMLIM */
    DEFAULT_PROCESS_LIMIT,      /* UL_GNPROC */
    DEFAULT_PROCESS_LIMIT,      /* UL_SNPROC */
    DEFAULT_FILE_LIMIT,         /* UL_GFILELIM */
    DEFAULT_FILE_LIMIT,         /* UL_SFILELIM */
    DEFAULT_CORE_LIMIT,         /* UL_GCORELIM */
    DEFAULT_CORE_LIMIT,         /* UL_SCORELIM */
    DEFAULT_DATA_LIMIT,         /* UL_GDATALIM */
    DEFAULT_DATA_LIMIT,         /* UL_SDATALIM */
    DEFAULT_STACK_LIMIT,        /* UL_GSTACKLIM */
    DEFAULT_STACK_LIMIT,        /* UL_SSTACKLIM */
};

/*
 * ulimit() - get and set user limits
 *
 * The ulimit() function provides control over process limits. This is a
 * legacy interface that predates the more comprehensive getrlimit/setrlimit
 * functions.
 *
 * Parameters:
 *   cmd: command to execute (UL_GETFSIZE, UL_SETFSIZE, etc.)
 *   ...: optional argument for set operations
 *
 * Returns: current limit value on success, -1 on error with errno set
 *
 * Note: On Amiga, many limits are approximated or set to reasonable
 * defaults since the system doesn't provide detailed resource tracking.
 */
long ulimit(int cmd, ...)
{
    va_list args;
    long new_limit;
    long result;
    
    /* Check for abort signal */
    __chkabort();
    
    /* Validate command */
    if (cmd < 1 || cmd > 18) {
        errno = EINVAL;
        return -1;
    }
    
    /* Handle get operations */
    if (cmd % 2 == 1) {  /* Odd commands are get operations */
        /* Initialize file size limit on first get if not set */
        if (cmd == UL_GETFSIZE && current_limits[0] == 0) {
            current_limits[0] = DEFAULT_FILE_SIZE_LIMIT;
        }
        
        result = current_limits[cmd - 1];
        return result;
    }
    
    /* Handle set operations */
    if (cmd % 2 == 0) {  /* Even commands are set operations */
        va_start(args, cmd);
        new_limit = va_arg(args, long);
        va_end(args);
        
        /* Validate new limit */
        if (new_limit < 0) {
            errno = EINVAL;
            return -1;
        }
        
        /* Set the limit */
        current_limits[cmd - 1] = new_limit;
        
        /* Return the new limit value */
        return new_limit;
    }
    
    /* Should not reach here */
    errno = EINVAL;
    return -1;
}
