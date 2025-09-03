/*
 * rcmd.c - routines for returning a stream to a remote command (POSIX compliant)
 *
 * The rcmd() function is used by the super-user to execute a command on
 * a remote machine using an authentication scheme based on reserved port
 * numbers. The rresvport() function returns a descriptor to a socket
 * with an address in the privileged port space.
 *
 * POSIX.1-2001, POSIX.1-2008, BSD 4.3
 */

#include "amiga.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

int rresvport(int *port)
{
    struct sockaddr_in sin;
    int s;
    int i;
    
    __chkabort();
    
    /* Create socket */
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        return -1;
    }
    
    /* Set up address structure */
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    
    /* Try to bind to a privileged port (512-1023) */
    for (i = 512; i < 1024; i++) {
        sin.sin_port = htons(i);
        if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) == 0) {
            if (port) {
                *port = i;
            }
            return s;
        }
    }
    
    /* If we get here, all privileged ports are in use */
    close(s);
    errno = EAGAIN;  /* All network ports in use */
    return -1;
}

int rcmd(char **ahost, int inport, const char *locuser, 
         const char *remuser, const char *cmd, int *fd2p)
{
    struct sockaddr_in sin;
    struct hostent *hp;
    int s, s2;
    char *host;
    int error;
    
    __chkabort();
    
    if (ahost == NULL || *ahost == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    host = *ahost;
    
    /* Look up the host using gethostbyname */
    hp = gethostbyname(host);
    if (hp == NULL) {
        return -1;
    }
    
    /* Set *ahost to the standard name of the host */
    *ahost = hp->h_name;
    
    /* Create socket */
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        return -1;
    }
    
    /* Set up address structure */
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(inport);
    memcpy(&sin.sin_addr, hp->h_addr, hp->h_length);
    
    /* Connect to remote host */
    if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
        error = errno;
        close(s);
        errno = error;
        return -1;
    }
    
    /* In a full implementation, we would:
     * 1. Send authentication information (locuser, remuser)
     * 2. Send the command to execute
     * 3. Set up stdin/stdout redirection
     * 4. Handle the second file descriptor if requested
     * 5. Set up auxiliary channel for control process if fd2p is non-zero
     */
    
    /* For now, this is a foundation implementation */
    if (fd2p) {
        *fd2p = -1;  /* No auxiliary channel implemented yet */
    }
    
    /* Return the socket file descriptor */
    return s;
}
