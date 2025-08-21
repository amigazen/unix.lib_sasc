/*
 * mktemp.c - create a unique temporary filename (POSIX compliant)
 *
 * This function generates a unique temporary filename from template.
 * The last six characters of template must be "XXXXXX" and these are
 * replaced with a string that makes the filename unique.
 *
 * POSIX.1-2001, POSIX.1-2008, 4.3BSD
 */

#include "amiga.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <proto/dos.h>
#include <dos/dostags.h>

char *mktemp(char *template)
{
    char *suffix;
    static int counter = 0;
    
    chkabort();
    
    if (template == NULL) {
        errno = EFAULT;
        return NULL;
    }
    
    /* Find the XXXXXX suffix */
    suffix = strstr(template, "XXXXXX");
    if (suffix == NULL) {
        errno = EINVAL;
        return NULL;
    }
    
    /* Generate unique filename */
    sprintf(suffix, "%06d", counter++);
    
    return template;
}
