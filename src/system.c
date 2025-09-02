/*
 * system.c - execute shell command (POSIX compliant)
 *
 * This function executes the command specified by string by calling
 * the host environment's command processor.
 *
 * POSIX.1-2001, POSIX.1-2008, C89, C99
 */

#include "amiga.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <proto/dos.h>
#include <dos/dostags.h>

/* Internal function for synchronous command execution */
static long doCommand(const char *command, BPTR other)
{
    struct TagItem stags[3];

    stags[0].ti_Tag = SYS_Input;
    stags[0].ti_Data = other ? other : Input();
    stags[1].ti_Tag = SYS_Output;
    stags[1].ti_Data = other ? 0L : Output();
    stags[2].ti_Tag = TAG_DONE;
    
    return System((char *)command, stags);
}

/*
 * system() - execute shell command
 *
 * The system() function shall cause the host environment to execute
 * the command pointed to by string.
 *
 * Returns: If command is a null pointer, system() shall return non-zero
 *          to indicate that a command processor is available, or zero if
 *          one is not available.
 *          If command is not a null pointer, system() shall return the
 *          termination status of the command language interpreter in an
 *          implementation-defined manner.
 */
int system(const char *command)
{
    long errcode = -1L;
    char *s;
    int sflag = 1;

    chkabort();

    /* If command is NULL, check if command processor is available */
    if (command == NULL) {
        /* On AmigaOS, we can check if System() is available */
        if (SysBase->lib_Version >= 36L) {
            return 1;  /* Command processor available */
        } else {
            return 0;  /* Command processor not available */
        }
    }

    /* Check if we should use System() or Execute() */
    if (SysBase->lib_Version < 36L) {
        sflag = 0;  /* Use Execute() for older systems */
    } else {
        /* Check SYSTEM environment variable */
        s = getenv("SYSTEM");
        if (s != NULL) {
            sflag = (*s == 'n') ? 0 : 1;
        }
    }

    if (sflag) {
        /* Use System() - more powerful, supports I/O redirection */
        errcode = doCommand(command, 0L);
    } else {
        /* Use Execute() - simpler, synchronous execution */
        long r = Execute((char *)command, (BPTR)0L, Output());
        if (r == -1L) {
            errcode = IoErr();
        } else {
            errcode = 0L;  /* Success */
        }
    }

    /* Convert AmigaOS return code to POSIX-style return value */
    if (errcode == -1L) {
        errno = EOSERR;
        return -1;
    }

    return (int)errcode;
}
