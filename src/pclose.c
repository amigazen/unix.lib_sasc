/*
 * pclose.c - close pipe stream to or from a process (POSIX compliant)
 *
 * This function closes a stream opened by popen() and waits for the
 * associated process to terminate.
 *
 * POSIX.1-2001, POSIX.1-2008, SVr4, 4.3BSD
 */

#include "amiga.h"
#include <stdio.h>
#include <errno.h>

int pclose(FILE *stream)
{
    chkabort();
    
    if (stream == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    /* Close the stream */
    if (fclose(stream) == 0) {
        return 0;
    } else {
        return -1;
    }
}
