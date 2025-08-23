/*
 * getgid.c - get group ID (POSIX compliant)
 *
 * This function returns the real group ID of the calling process.
 * On AmigaOS, this always returns 0 (root) as there's no group system.
 *
 * POSIX.1-2001, POSIX.1-2008, SVr4, 4.3BSD
 */

#include "amiga.h"
#include <unistd.h>

gid_t getgid(void)
{
    chkabort();
    
    /* On AmigaOS, there's no group system, so always return root */
    return 0;
}
