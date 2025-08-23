/*
 * glob.c - pathname pattern matching (POSIX compliant)
 *
 * This function searches for all the pathnames matching pattern
 * according to the rules used by the shell. On AmigaOS, this is
 * a simplified implementation.
 *
 * POSIX.1-2001, POSIX.1-2008, 4.3BSD
 */

#include "amiga.h"
#include <glob.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

int glob(const char *pattern, int flags, int (*errfunc)(const char *, int), glob_t *pglob)
{
    chkabort();
    
    if (pattern == NULL || pglob == NULL) {
        errno = EFAULT;
        return GLOB_NOMATCH;
    }
    
    /* Initialize glob structure */
    pglob->gl_pathc = 0;
    pglob->gl_pathv = NULL;
    pglob->gl_offs = 0;
    
    /* On AmigaOS, we don't implement full glob functionality */
    /* This is a placeholder that returns no matches */
    errno = ENOSYS;
    return GLOB_NOMATCH;
}

void globfree(glob_t *pglob)
{
    int i;
    
    if (pglob == NULL) {
        return;
    }
    
    /* Free allocated strings */
    if (pglob->gl_pathv != NULL) {
        for (i = 0; i < pglob->gl_pathc; i++) {
            if (pglob->gl_pathv[i] != NULL) {
                free(pglob->gl_pathv[i]);
            }
        }
        free(pglob->gl_pathv);
    }
    
    /* Reset structure */
    pglob->gl_pathc = 0;
    pglob->gl_pathv = NULL;
    pglob->gl_offs = 0;
}
