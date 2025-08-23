/*
 * realpath.c - resolve a pathname (POSIX compliant)
 *
 * This function expands all symbolic links and resolves references to
 * /./, /../ and extra '/' characters in the null-terminated string
 * named by path to produce a canonicalized absolute pathname.
 *
 * POSIX.1-2001, POSIX.1-2008, 4.4BSD
 */

#include "amiga.h"
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <proto/dos.h>
#include <dos/dostags.h>

char *realpath(const char *path, char *resolved_path)
{
    char *result;
    BPTR lock;
    struct FileInfoBlock *fib;
    
    chkabort();
    
    if (path == NULL) {
        errno = EFAULT;
        return NULL;
    }
    
    /* Allocate buffer if not provided */
    if (resolved_path == NULL) {
        resolved_path = malloc(PATH_MAX);
        if (resolved_path == NULL) {
            return NULL;
        }
    }
    
    /* Try to lock the path */
    lock = Lock(path, ACCESS_READ);
    if (lock == NULL) {
        if (resolved_path != NULL) {
            free(resolved_path);
        }
        errno = convert_oserr(IoErr());
        return NULL;
    }
    
    /* Get file information */
    fib = AllocDosObjectTags(DOS_FIB, TAG_END);
    if (fib == NULL) {
        UnLock(lock);
        if (resolved_path != NULL) {
            free(resolved_path);
        }
        errno = ENOMEM;
        return NULL;
    }
    
    /* Examine the file */
    if (Examine(lock, fib)) {
        /* Get the full path */
        if (NameFromLock(lock, resolved_path, PATH_MAX)) {
            UnLock(lock);
            FreeDosObject(DOS_FIB, fib);
            return resolved_path;
        }
    }
    
    /* Cleanup on failure */
    UnLock(lock);
    FreeDosObject(DOS_FIB, fib);
    if (resolved_path != NULL) {
        free(resolved_path);
    }
    
    errno = EIO;
    return NULL;
}
