/*
 * unsetenv.c - remove environment variable (POSIX compliant)
 *
 * This function removes the variable name from the environment.
 * If name does not exist in the environment, then the function
 * succeeds, and the environment is unchanged.
 *
 * POSIX.1-2001, POSIX.1-2008, 4.3BSD
 */

#include "amiga.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <proto/dos.h>
#include <dos/dostags.h>

int unsetenv(const char *name)
{
    chkabort();
    
    if (name == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    /* Remove the environment variable using AmigaDOS */
    if (DeleteVar(name, GVF_GLOBAL_VAR)) {
        return 0;
    } else {
        /* Don't treat "variable not found" as an error */
        if (IoErr() == ERROR_OBJECT_NOT_FOUND) {
            return 0;
        }
        
        errno = convert_oserr(IoErr());
        return -1;
    }
}
