/*
 * uname.c - get system name (POSIX compliant)
 *
 * This function returns system information in the structure pointed to by name.
 * The uname() function returns information identifying the current system.
 *
 * POSIX.1-2001, POSIX.1-2008, SVr4, 4.3BSD
 */

#include "amiga.h"
#include <sys/utsname.h>
#include <string.h>
#include <errno.h>
#include <proto/exec.h>

int uname(struct utsname *name)
{
    LONG ver_major, ver_minor;
    chkabort();
    
    if (name == NULL) {
        errno = EFAULT;
        return -1;
    }
    
    /* Set system name */
    strcpy(name->sysname, "AmigaOS");
    
    /* Set machine architecture */
    strcpy(name->machine, "m68k");
    
    /* Set node name (hostname) */
    strcpy(name->nodename, "localhost");
    
    /* Set release version */
    ver_major = SysBase->LibNode.lib_Version;
    ver_minor = SysBase->SoftVer;
    sprintf(name->release, "%d.%d", ver_major, ver_minor);
    
    /* Set version string */
    if (ver_major < 36) {
        strcpy(name->version, "1");
    } else if (ver_major < 39) {
        strcpy(name->version, "2");
    } else if (ver_major < 50) {
        strcpy(name->version, "3");
    } else {
        strcpy(name->version, "4");
    }

    
    return 0;
}
