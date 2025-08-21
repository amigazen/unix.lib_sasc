/*
 * setenv.c - set environment variable (POSIX compliant)
 *
 * This function adds the variable name to the environment with the
 * value value, if name does not already exist. If name does exist
 * in the environment, then its value is changed to value if overwrite
 * is nonzero; if overwrite is zero, then the value of name is not
 * changed.
 *
 * POSIX.1-2001, POSIX.1-2008, 4.3BSD
 */

#include "amiga.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <proto/dos.h>
#include <dos/dos.h>
#include <dos/dostags.h>

int setenv(const char *name, const char *value, int overwrite)
{
    char *env_string;
    size_t len;
    
    chkabort();
    
    if (name == NULL || value == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    /* Check if variable already exists */
    if (!overwrite && getenv(name) != NULL) {
        return 0;  /* Variable exists and we're not overwriting */
    }
    
    /* Create environment string in format "name=value" */
    len = strlen(name) + strlen(value) + 2;  /* +2 for '=' and '\0' */
    env_string = malloc(len);
    if (env_string == NULL) {
        errno = ENOMEM;
        return -1;
    }
    
    sprintf(env_string, "%s=%s", name, value);
    
    /* Set the environment variable using AmigaDOS */
    if (SetVar(name, value, -1, GVF_GLOBAL_VAR)) {
        free(env_string);
        return 0;
    } else {
        free(env_string);
        errno = convert_oserr(IoErr());
        return -1;
    }
}
