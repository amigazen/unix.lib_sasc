/*
 * setgid.c - set group ID (POSIX compliant)
 *
 * This function sets the effective group ID of the calling process.
 * On AmigaOS, this is a no-op as there's no group system.
 *
 * POSIX.1-2001, POSIX.1-2008, SVr4, 4.3BSD
 */

#include "amiga.h"
#include <unistd.h>
#include <errno.h>

int setgid(gid_t gid)
{
    chkabort();
    
    /* On AmigaOS, there's no group system, so this is a no-op */
    /* Return success to maintain compatibility */
    return 0;
}
