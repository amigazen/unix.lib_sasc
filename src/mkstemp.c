/*
 * mkstemp.c - create a unique temporary file (POSIX compliant)
 *
 * This function generates a unique temporary filename from template,
 * creates and opens the file, and returns an open file descriptor
 * for the file.
 *
 * POSIX.1-2001, POSIX.1-2008, 4.3BSD
 */

#include "amiga.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <proto/dos.h>
#include <dos/dostags.h>

int mkstemp(char *template)
{
    char *suffix;
    int fd;
    static int counter = 0;
    
    chkabort();
    
    if (template == NULL) {
        errno = EFAULT;
        return -1;
    }
    
    /* Find the XXXXXX suffix */
    suffix = strstr(template, "XXXXXX");
    if (suffix == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    /* Generate unique filename */
    do {
        sprintf(suffix, "%06d", counter++);
        
        /* Try to create the file */
        fd = open(template, O_CREAT | O_EXCL | O_RDWR, 0600);
        
        if (fd >= 0) {
            /* Success - unlink the file so it's removed when closed */
            unlink(template);
            return fd;
        }
        
        /* If file exists, try next number */
        if (errno != EEXIST) {
            return -1;
        }
        
    } while (counter < 1000000);  /* Prevent infinite loop */
    
    errno = EEXIST;
    return -1;
}
