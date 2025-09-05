/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * util.c - POSIX utility functions
 * 
 * This file implements POSIX utility functions for terminal handling
 * and process management.
 * 
 * Functions implemented:
 * - login_tty() - Make terminal the controlling terminal
 * - forkpty() - Fork with pseudo-terminal (using vfork())
 * - openpty() - Open pseudo-terminal
 * 
 * C89 compliant for AmigaOS compatibility
 */

#include "amiga.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>

/* Forward declarations */
static int _openpty(int *amaster, int *aslave, char *name, const struct termios *termp, const struct winsize *winp);

/*
 * login_tty - Make terminal the controlling terminal
 * 
 * This function makes the given file descriptor the controlling terminal
 * for the current process.
 * 
 * @param fd File descriptor of the terminal
 * @return 0 on success, -1 on error
 */
int login_tty(int fd)
{
    /* Check for abort signal */
    __chkabort();
    
    /* Validate file descriptor */
    if (fd < 0) {
        errno = EBADF;
        return -1;
    }
    
    /* On AmigaOS, we can't really change the controlling terminal */
    /* This is a simplified implementation that just checks if fd is valid */
    if (isatty(fd)) {
        return 0;
    } else {
        errno = ENOTTY;
        return -1;
    }
}

/*
 * forkpty - Fork with pseudo-terminal
 * 
 * This function creates a pseudo-terminal and forks a child process using vfork().
 * The child process has the pseudo-terminal as its controlling terminal.
 * Note: Uses vfork() instead of fork() as AmigaOS doesn't support fork().
 * 
 * @param amaster Pointer to store master file descriptor
 * @param name Pointer to store terminal name (can be NULL)
 * @param termp Terminal attributes (can be NULL)
 * @param winp Window size (can be NULL)
 * @return Child process ID on success, -1 on error
 */
pid_t forkpty(int *amaster, char *name, const struct termios *termp, const struct winsize *winp)
{
    pid_t pid;
    int aslave;
    
    /* Check for abort signal */
    __chkabort();
    
    /* Validate parameters */
    if (amaster == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    /* Open pseudo-terminal */
    if (_openpty(amaster, &aslave, name, termp, winp) < 0) {
        return -1;
    }
    
    /* Fork child process using vfork() */
    pid = vfork();
    if (pid < 0) {
        /* vfork failed, close file descriptors */
        close(*amaster);
        close(aslave);
        return -1;
    }
    
    if (pid == 0) {
        /* Child process */
        close(*amaster);  /* Close master in child */
        
        /* Make slave the controlling terminal */
        if (login_tty(aslave) < 0) {
            close(aslave);
            _exit(1);
        }
        
        /* Set standard file descriptors to slave */
        dup2(aslave, STDIN_FILENO);
        dup2(aslave, STDOUT_FILENO);
        dup2(aslave, STDERR_FILENO);
        
        if (aslave > 2) {
            close(aslave);
        }
    } else {
        /* Parent process */
        close(aslave);  /* Close slave in parent */
    }
    
    return pid;
}

/*
 * openpty - Open pseudo-terminal
 * 
 * This function opens a pseudo-terminal and returns file descriptors
 * for the master and slave ends.
 * 
 * @param amaster Pointer to store master file descriptor
 * @param aslave Pointer to store slave file descriptor
 * @param name Pointer to store terminal name (can be NULL)
 * @param termp Terminal attributes (can be NULL)
 * @param winp Window size (can be NULL)
 * @return 0 on success, -1 on error
 */
int openpty(int *amaster, int *aslave, char *name, const struct termios *termp, const struct winsize *winp)
{
    /* Check for abort signal */
    __chkabort();
    
    /* Validate parameters */
    if (amaster == NULL || aslave == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    return _openpty(amaster, aslave, name, termp, winp);
}

/*
 * _openpty - Internal function to open pseudo-terminal
 * 
 * This function implements the actual pseudo-terminal opening logic.
 * On AmigaOS, we simulate this using regular file descriptors.
 * 
 * @param amaster Pointer to store master file descriptor
 * @param aslave Pointer to store slave file descriptor
 * @param name Pointer to store terminal name (can be NULL)
 * @param termp Terminal attributes (can be NULL)
 * @param winp Window size (can be NULL)
 * @return 0 on success, -1 on error
 */
static int _openpty(int *amaster, int *aslave, char *name, const struct termios *termp, const struct winsize *winp)
{
    int master_fd, slave_fd;
    
    /* On AmigaOS, we don't have real pseudo-terminals */
    /* We'll simulate this using regular file descriptors */
    
    /* Open master (simulate with a regular file) */
    master_fd = open("CON:0/0/640/200/Pseudo Terminal", O_RDWR);
    if (master_fd < 0) {
        return -1;
    }
    
    /* Open slave (simulate with another file descriptor) */
    slave_fd = dup(master_fd);
    if (slave_fd < 0) {
        close(master_fd);
        return -1;
    }
    
    /* Store file descriptors */
    *amaster = master_fd;
    *aslave = slave_fd;
    
    /* Store terminal name if requested */
    if (name != NULL) {
        strcpy(name, "CON:0/0/640/200/Pseudo Terminal");
    }
    
    /* Set terminal attributes if requested */
    if (termp != NULL) {
        /* On AmigaOS, we can't really set terminal attributes */
        /* This is a no-op for compatibility */
    }
    
    /* Set window size if requested */
    if (winp != NULL) {
        /* On AmigaOS, we can't really set window size */
        /* This is a no-op for compatibility */
    }
    
    return 0;
}
