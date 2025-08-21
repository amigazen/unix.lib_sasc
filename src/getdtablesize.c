/*
 * getdtablesize.c - get file descriptor table size (POSIX compliant)
 *
 * This function returns the maximum number of file descriptors that
 * a process can have open at one time.
 *
 * POSIX.1-2001, POSIX.1-2008, 4.3BSD
 */

#include "amiga.h"
#include <unistd.h>
#include <limits.h>

int getdtablesize(void)
{
    chkabort();
    
    /* Return a reasonable limit for AmigaOS */
    /* This could be made configurable or determined at runtime */
    return 256;
}
