/*
 * readlink.c - read value of a symbolic link (POSIX compliant)
 *
 * This function places the contents of the symbolic link path in the
 * buffer buf, which has size bufsiz. The readlink() function does not
 * append a null byte to buf.
 *
 * POSIX.1-2001, POSIX.1-2008, SVr4, 4.3BSD
 */

#include "amiga.h"
#include <errno.h>
#include <proto/dos.h>
#include <dos/dostags.h>
#include <string.h>
#include <unistd.h>

ssize_t readlink(const char *path, char *buf, size_t bufsiz)
{
    struct MsgPort *port;
    BPTR dirlock, olddir;
    char *p, *linkname;
    char c;
    ssize_t result;
    
    chkabort();
    
    if (path == NULL || buf == NULL) {
        errno = EFAULT;
        return -1;
    }
    
    if (bufsiz == 0) {
        return 0;
    }
    
    /* Get device port for the path */
    port = DeviceProc(path);
    if (port == NULL) {
        errno = EIO;
        return -1;
    }
    
    /* Split path into directory and filename */
    p = PathPart(path);
    if (p == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    linkname = FilePart(path);
    if (linkname == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    c = *p;
    *p = '\0';
    
    /* Lock the directory */
    dirlock = Lock(path, ACCESS_READ);
    if (dirlock) {
        olddir = CurrentDir(dirlock);
        
        /* Read the symbolic link */
        if (ReadLink(port, dirlock, linkname, buf, bufsiz)) {
            CurrentDir(olddir);
            UnLock(dirlock);
            
            /* Restore path */
            *p = c;
            
            /* Return length of link content */
            result = strlen(buf);
            if (result >= bufsiz) {
                /* Buffer was too small, truncate result */
                result = bufsiz - 1;
            }
            return result;
        } else {
            CurrentDir(olddir);
            UnLock(dirlock);
            errno = convert_oserr(IoErr());
        }
        
        /* Restore path */
        *p = c;
    } else {
        errno = convert_oserr(IoErr());
    }
    
    return -1;
}
