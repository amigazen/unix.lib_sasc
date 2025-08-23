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
#include <dos/var.h>

int unsetenv(const char *name)
{
    LONG error_code;
    chkabort();
    
    if (name == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    /* Remove the environment variable using AmigaDOS */
    if (DeleteVar(name, GVF_GLOBAL_ONLY)) {
        return 0;
    } else {
        /* Get the error code */
        error_code = IoErr();
        
        /* Don't treat "variable not found" as an error */
        /* On AmigaOS, this typically means the variable didn't exist */
        if (error_code == 0 || error_code == 205) { /* 205 = ERROR_OBJECT_NOT_FOUND */
            return 0;
        }
        
        errno = convert_oserr(error_code);
        return -1;
    }
}
