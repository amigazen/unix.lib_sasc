#include "amiga.h"
#include <sys/param.h>
#include <proto/dos.h>
#include <errno.h>

char *getwd(char *pathname)
{
    BPTR lock;
    char *result;
    
    __chkabort();
    
    if (pathname == NULL) {
	/* Allocate buffer if not provided */
	pathname = malloc(MAXPATHLEN);
	if (pathname == NULL) {
	    return NULL;
	}
    }
    
    /* Get current directory lock */
    lock = GetCurrentDir();
    if (lock == NULL) {
	/* NULL means root directory */
	strcpy(pathname, "/");
	return pathname;
    }
    
    /* Get pathname from lock */
    if (NameFromLock(lock, pathname, MAXPATHLEN)) {
	return pathname;
    } else {
	errno = convert_oserr(IoErr());
	return NULL;
    }
}
