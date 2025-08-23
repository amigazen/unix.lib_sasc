/*
 * getuid.c - get user ID (POSIX compliant)
 *
 * This function returns the real user ID of the calling process.
 * On AmigaOS, this always returns 0 (root) as there's no user system.
 *
 * POSIX.1-2001, POSIX.1-2008, SVr4, 4.3BSD
 */

#include "amiga.h"
#include <unistd.h>

uid_t getuid(void)
{
    chkabort();
    
    /* On AmigaOS, there's no user system, so always return root */
    return 0;
}
