/*
 * basename.c - extract filename from path (POSIX compliant)
 *
 * This function returns a pointer to the filename component of path.
 * The returned string is statically allocated and should not be freed.
 *
 * POSIX.1-2001, POSIX.1-2008, 4.3BSD
 */

#include "amiga.h"
#include "libgen.h"
#include <string.h>

char *basename(char *path)
{
    static char result[256];
    char *filename;
    
    chkabort();
    
    if (path == NULL || *path == '\0') {
        strcpy(result, ".");
        return result;
    }
    
    /* Find the last component of the path */
    filename = strrchr(path, '/');
    if (filename == NULL) {
        filename = strrchr(path, ':');
    }
    
    if (filename == NULL) {
        /* No directory separator found */
        strcpy(result, path);
    } else {
        /* Skip the separator */
        filename++;
        if (*filename == '\0') {
            /* Path ends with separator */
            strcpy(result, ".");
        } else {
            strcpy(result, filename);
        }
    }
    
    return result;
}
