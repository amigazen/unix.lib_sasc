/*
 * dirname.c - extract directory from path (POSIX compliant)
 *
 * This function returns a pointer to the directory component of path.
 * The returned string is statically allocated and should not be freed.
 *
 * POSIX.1-2001, POSIX.1-2008, 4.3BSD
 */

#include "amiga.h"
#include <libgen.h>
#include <string.h>

char *dirname(char *path)
{
    static char result[256];
    char *last_sep;
    
    chkabort();
    
    if (path == NULL || *path == '\0') {
        strcpy(result, ".");
        return result;
    }
    
    /* Find the last directory separator */
    last_sep = strrchr(path, '/');
    if (last_sep == NULL) {
        last_sep = strrchr(path, ':');
    }
    
    if (last_sep == NULL) {
        /* No directory separator found */
        strcpy(result, ".");
    } else if (last_sep == path) {
        /* Path starts with separator */
        if (*(last_sep + 1) == '\0') {
            strcpy(result, "/");
        } else {
            strcpy(result, "/");
        }
    } else {
        /* Copy directory part */
        strncpy(result, path, last_sep - path);
        result[last_sep - path] = '\0';
        
        /* Handle root directory case */
        if (strlen(result) == 0) {
            strcpy(result, "/");
        }
    }
    
    return result;
}
