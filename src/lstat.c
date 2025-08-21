/*
 * lstat.c - get file status without following symbolic links (POSIX compliant)
 *
 * This function is identical to stat(), except that if path is a
 * symbolic link, then the link itself is stat-ed, not the file
 * that it refers to.
 *
 * POSIX.1-2001, POSIX.1-2008, 4.3BSD
 */

#include "amiga.h"
#include <sys/stat.h>
#include <errno.h>
#include <proto/dos.h>
#include <dos/dostags.h>

int lstat(const char *path, struct stat *buf)
{
    struct FileInfoBlock *fib;
    BPTR lock;
    int ret;
    
    chkabort();
    
    if (path == NULL || buf == NULL) {
        errno = EFAULT;
        return -1;
    }
    
    /* Allocate file info block */
    if ((fib = AllocDosObjectTags(DOS_FIB, TAG_END)) == NULL) {
        errno = ENOMEM;
        return -1;
    }
    
    /* Lock the path */
    if ((lock = Lock(path, ACCESS_READ)) == NULL) {
        FreeDosObject(DOS_FIB, fib);
        errno = convert_oserr(IoErr());
        return -1;
    }
    
    /* Examine the file */
    if (Examine(lock, fib)) {
        BPTR parent = ParentDir(lock);
        int isroot = !parent;
        
        if (parent) UnLock(parent);
        
        /* Convert FIB to stat structure */
        _fibstat(fib, isroot, buf);
        ret = 0;
    } else {
        ret = -1;
        errno = convert_oserr(IoErr());
    }
    
    /* Cleanup */
    UnLock(lock);
    FreeDosObject(DOS_FIB, fib);
    
    return ret;
}
