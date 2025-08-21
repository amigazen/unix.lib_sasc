/*
 * sysconf.c - get system configuration values (POSIX compliant)
 *
 * This function provides a method for applications to determine the
 * current value of a configurable system limit or option variable.
 *
 * POSIX.1-2001, POSIX.1-2008, SVr4, 4.3BSD
 */

#include "amiga.h"
#include <unistd.h>
#include <errno.h>
#include <limits.h>

/* System configuration constants */
#define _SC_ARG_MAX           1
#define _SC_CHILD_MAX         2
#define _SC_CLK_TCK           3
#define _SC_NGROUPS_MAX       4
#define _SC_OPEN_MAX          5
#define _SC_STREAM_MAX        6
#define _SC_TZNAME_MAX        7
#define _SC_JOB_CONTROL       8
#define _SC_SAVED_IDS         9
#define _SC_VERSION           10
#define _SC_PAGESIZE          11
#define _SC_PAGE_SIZE         12
#define _SC_XOPEN_VERSION     13

long sysconf(int name)
{
    chkabort();
    
    switch (name) {
        case _SC_ARG_MAX:
            return 4096;  /* Reasonable limit for AmigaOS */
            
        case _SC_CHILD_MAX:
            return 32;    /* AmigaOS process limit */
            
        case _SC_CLK_TCK:
            return 50;    /* AmigaOS ticks per second (PAL) */
            
        case _SC_NGROUPS_MAX:
            return 0;     /* AmigaOS doesn't support groups */
            
        case _SC_OPEN_MAX:
            return 256;   /* Reasonable file descriptor limit */
            
        case _SC_STREAM_MAX:
            return 256;   /* Reasonable stream limit */
            
        case _SC_TZNAME_MAX:
            return 64;    /* Timezone name limit */
            
        case _SC_JOB_CONTROL:
            return -1;    /* Not supported on AmigaOS */
            
        case _SC_SAVED_IDS:
            return -1;    /* Not supported on AmigaOS */
            
        case _SC_VERSION:
            return 200112L; /* POSIX.1-2001 */
            
        case _SC_PAGESIZE:
        case _SC_PAGE_SIZE:
            return 4096;  /* Typical page size */
            
        case _SC_XOPEN_VERSION:
            return 4;     /* X/Open System Interfaces */
            
        default:
            errno = EINVAL;
            return -1;
    }
}
