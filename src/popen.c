/*
 * popen.c - pipe stream to or from a process (POSIX compliant)
 *
 * This function opens a process by creating a pipe, forking, and
 * invoking the shell. Since a pipe is by definition unidirectional,
 * the type argument may specify only reading or writing, not both.
 *
 * POSIX.1-2001, POSIX.1-2008, SVr4, 4.3BSD
 */

#include "amiga.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <proto/dos.h>
#include <dos/dostags.h>

FILE *popen(const char *command, const char *type)
{
    char filename[256];
    FILE *stream;
    BPTR filehandle;
    LONG mode;
    static int counter = 1;
    BPTR console;
    
    chkabort();
    
    if (command == NULL || type == NULL) {
        errno = EINVAL;
        return NULL;
    }
    
    /* Validate type parameter */
    if ((type[0] != 'r' && type[0] != 'w') || type[1] != '\0') {
        errno = EINVAL;
        return NULL;
    }
    
    /* Create unique pipe filename */
    sprintf(filename, "PIPE:unixlib_%ld_%ld", counter++, FindTask(0));
    
    /* Open the pipe file */
    if (type[0] == 'r') {
        mode = MODE_NEWFILE;  /* Read mode - peer must write */
    } else {
        mode = MODE_OLDFILE;  /* Write mode - peer must read */
    }
    
    stream = fopen(filename, type);
    if (stream == NULL) {
        errno = ENOENT;
        return NULL;
    }
    
    /* Open the pipe with AmigaDOS */
    filehandle = Open(filename, mode);
    if (filehandle == NULL) {
        fclose(stream);
        errno = ENOENT;
        return NULL;
    }
    
    /* Execute the command  ToDo: change the asterisk to CON:*/
    console;
    if (type[0] == 'r') {
        /* Read mode: execute command with output to pipe */
        console = Open("*", MODE_OLDFILE);
        if (console && SystemTags(command, SYS_Asynch, TRUE, 
                                 SYS_Output, filehandle, 
                                 SYS_Input, console, TAG_DONE) == 0) {
            Close(filehandle);
            return stream;
        }
    } else {
        /* Write mode: execute command with input from pipe */
        console = Open("*", MODE_NEWFILE);
        if (console && SystemTags(command, SYS_Asynch, TRUE, 
                                 SYS_Input, filehandle, 
                                 SYS_Output, console, TAG_DONE) == 0) {
            Close(filehandle);
            return stream;
        }
    }
    
    /* Cleanup on failure */
    fclose(stream);
    Close(filehandle);
    if (console) Close(console);
    
    errno = EAGAIN;
    return NULL;
}
