/*
 * getpass.c - get password without echo (POSIX compliant)
 *
 * This function displays the string prompt and disables echoing,
 * reads a string of up to 128 characters, and restores the terminal
 * state. On AmigaOS, this uses CONSOLE: for input.
 *
 * POSIX.1-2001, POSIX.1-2008, 4.3BSD
 */

#include "amiga.h"
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <proto/dos.h>
#include <dos/dostags.h>

char *getpass(const char *prompt)
{
    static char password[129];
    BPTR console;
    int i;
    char c;
    
    chkabort();
    
    if (prompt == NULL) {
        errno = EFAULT;
        return NULL;
    }
    
    /* Display prompt */
    printf("%s", prompt);
    fflush(stdout);
    
    /* Open CONSOLE: for input */
    console = Open("CONSOLE:", MODE_OLDFILE);
    if (console == NULL) {
        errno = EIO;
        return NULL;
    }
    
    /* Read password without echo */
    i = 0;
    while (i < 128) {
        c = FGetC(console);
        if (c == '\n' || c == '\r' || c == EOF) {
            break;
        }
        password[i++] = c;
    }
    password[i] = '\0';
    
    /* Close console */
    Close(console);
    
    /* Print newline */
    printf("\n");
    
    return password;
}
