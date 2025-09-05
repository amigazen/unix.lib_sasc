/*
 * chdir.c - change directory (POSIX compliant)
 *
 * This function changes the current working directory to the specified path.
 * Uses AmigaOS dos.library GetCurrentDir() for proper directory handling.
 *
 * POSIX.1-2001, POSIX.1-2008
 */

#include "amiga.h"
#include <proto/dos.h>
#include <errno.h>

int chdir(const char *path)
{
    BPTR new_lock;
    BPTR old_lock;
    
    __chkabort();
    
    if (path == NULL) {
	errno = EFAULT;
	return -1;
    }
    
    /* Try to lock the new directory */
    new_lock = Lock(path, SHARED_LOCK);
    if (new_lock == NULL) {
	errno = convert_oserr(IoErr());
	return -1;
    }
    
    /* Get current directory lock */
    old_lock = GetCurrentDir();
    
    /* Change to new directory */
    if (CurrentDir(new_lock)) {
	/* Success - unlock the new lock since CurrentDir takes ownership */
	/* Don't unlock old_lock as it's managed by the system */
	return 0;
    } else {
	/* Failed to change directory - restore old directory */
	CurrentDir(old_lock);
	UnLock(new_lock);
	errno = convert_oserr(IoErr());
	return -1;
    }
}
