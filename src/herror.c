/*
 * herror.c - print name resolver error message to stderr (POSIX compliant)
 *
 * This function finds the error message corresponding to the current value
 * of host error and writes it to stderr. If the argument string is non-NULL
 * it is used as a prefix to the message string.
 *
 * POSIX.1-2001, POSIX.1-2008, BSD 4.3
 */

#include "amiga.h"
#include <stdio.h>
#include <string.h>
#include <netdb.h>

/* External h_errno variable for network errors */
extern int h_errno;

/* Network error messages - these should match netdb.h constants */
static const char *h_errlist[] = {
    "Resolver Error 0 (no error)",           /* NETDB_SUCCESS = 0 */
    "Unknown host",                          /* HOST_NOT_FOUND = 1 */
    "Host name lookup failure",              /* TRY_AGAIN = 2 */
    "Unknown server error",                  /* NO_RECOVERY = 3 */
    "No address associated with name",       /* NO_DATA = 4 */
    "Unknown error"                          /* Unknown error code */
};

static const int h_nerr = sizeof(h_errlist) / sizeof(h_errlist[0]);

void herror(const char *s)
{
    __chkabort();
    
    /* Print prefix if provided */
    if (s && *s) {
        fprintf(stderr, "%s: ", s);
    }
    
    /* Print the appropriate error message */
    if (h_errno >= 0 && h_errno < h_nerr) {
        fprintf(stderr, "%s\n", h_errlist[h_errno]);
    } else {
        fprintf(stderr, "%s\n", h_errlist[h_nerr - 1]);
    }
}
