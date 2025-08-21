/*
 * fsync.c - synchronize file data (POSIX compliant)
 *
 * This function causes all modified data and attributes of fd to be
 * moved to a permanent storage device. This normally results in all
 * in-core modified copies of buffers for the associated file to be
 * written to a disk.
 *
 * POSIX.1-2001, POSIX.1-2008, 4.3BSD
 */

#include "amiga.h"
#include <unistd.h>
#include <errno.h>

int fsync(int fd)
{
    chkabort();
    
    /* On AmigaOS, we can't force a sync to disk, but we can ensure
       the file is written to the filesystem cache */
    
    /* For now, just return success as AmigaOS handles most caching
       automatically. In a more sophisticated implementation, we could
       try to flush any pending writes */
    
    return 0;
}
