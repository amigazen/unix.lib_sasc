/*
 * getlogin.c - get login name (POSIX compliant)
 *
 * This function returns a pointer to a string containing the name
 * of the user logged in on the controlling terminal of the process.
 * On AmigaOS, this returns a default value.
 *
 * POSIX.1-2001, POSIX.1-2008, SVr4, 4.3BSD
 */

#include "amiga.h"
#include <unistd.h>
#include <string.h>

char *getlogin(void)
{
    chkabort();
    
    /* On AmigaOS, there's no login system, so return a default */
    /* This could be made configurable via environment variables */
    return "amiga"; /* TODO: make this use the environment variable USER */
}
