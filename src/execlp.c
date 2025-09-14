/*
 * SPDX-License-Identifier: BSD-2-Clause
 * execlp.c - execute file with PATH search (POSIX compliant)
 *
 * Based on public domain tar implementation for Amiga by John Gilmore
 * Copyright (C) 2025 by amigazen project
 */

#include "amiga.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dos/dos.h>
#include <dos/dosextens.h>

extern char **environ;

/* Amiga-specific path component structure */
struct PathComponent {
    BPTR pc_Next;
    BPTR pc_Lock;
};

/**
 * @brief Get Amiga command path from CLI structure
 * @param pathbuf Buffer to store path string
 * @param bufsize Size of path buffer
 * @return 1 if path found, 0 if not available
 * 
 * Reads the command path from the Amiga CLI structure's cli_CommandDir
 * and converts it to a colon-separated string like Unix PATH.
 */
static int get_amiga_path(char *pathbuf, int bufsize)
{
    struct CommandLineInterface *cli;
    struct PathComponent *pc;
    char *buf = pathbuf;
    int remaining = bufsize - 1;
    int first = 1;
    
    /* Get CLI structure */
    cli = (struct CommandLineInterface *)Cli();
    if (!cli) {
        return 0;
    }
    
    /* Start with current directory (always first in Amiga path) */
    if (remaining > 0) {
        char curdir[256];
        if (GetCurrentDirName(curdir, sizeof(curdir))) {
            int len = strlen(curdir);
            if (len < remaining) {
                strcpy(buf, curdir);
                buf += len;
                remaining -= len;
                first = 0;
            }
        } else {
            /* Fallback to current directory lock method */
            BPTR lock = GetCurrentDir();
            if (lock && NameFromLock(lock, curdir, sizeof(curdir))) {
                int len = strlen(curdir);
                if (len < remaining) {
                    strcpy(buf, curdir);
                    buf += len;
                    remaining -= len;
                    first = 0;
                }
            }
        }
    }
    
    /* Walk through path components */
    pc = (struct PathComponent *)cli->cli_CommandDir;
    while (pc && remaining > 0) {
        char dirname[256];
        BPTR lock = pc->pc_Lock;
        
        if (lock && NameFromLock(lock, dirname, sizeof(dirname))) {
            int len = strlen(dirname);
            if (!first && remaining > 1) {
                *buf++ = ':';
                remaining--;
            }
            if (len < remaining) {
                strcpy(buf, dirname);
                buf += len;
                remaining -= len;
                first = 0;
            }
        }
        pc = (struct PathComponent *)pc->pc_Next;
    }
    
    /* Add C: directory (always last in Amiga path) */
    if (remaining > 2) {
        if (!first) {
            *buf++ = ':';
            remaining--;
        }
        if (remaining > 1) {
            strcpy(buf, "C:");
            remaining -= 2;
        }
    }
    
    *buf = '\0';
    return 1;
}

/**
 * @brief Execute file with PATH search
 * @param filename Name of file to execute
 * @param arg0 First argument (program name)
 * @param ... Additional arguments, terminated by NULL
 * @return -1 on error, does not return on success
 * 
 * The execlp() function searches for the executable file in the directories
 * specified by the Amiga CLI command path (cli_CommandDir). If no CLI is
 * available, it attempts to execute the file directly. If the file is found
 * but not executable as a binary, it attempts to execute it as a shell script.
 * 
 * This implementation provides comprehensive PATH search with proper error
 * handling and shell script execution fallback.
 */
int execlp(const char *filename, const char *arg0, ...)
{
    const char *path;
    char *path_copy;
    char *path_ptr;
    char *colon;
    struct stat statbuf;
    char **argstart;
    va_list args;
    int argc;
    int i;
    char *shell;
    char *arg;
    
    /* Count arguments to determine array size needed */
    va_start(args, arg0);
    argc = 1; /* arg0 */
    while ((arg = va_arg(args, char *)) != NULL) {
        argc++;
    }
    va_end(args);
    
    /* Allocate argument array */
    argstart = malloc((argc + 1) * sizeof(char *));
    if (argstart == NULL) {
        errno = ENOMEM;
        return -1;
    }
    
    /* Fill argument array */
    va_start(args, arg0);
    argstart[0] = (char *)arg0;
    for (i = 1; i < argc; i++) {
        argstart[i] = va_arg(args, char *);
    }
    argstart[argc] = NULL;
    va_end(args);
    
    /* Get Amiga command path from CLI structure */
    path_copy = malloc(1024); /* Large enough for typical Amiga paths */
    if (path_copy == NULL) {
        free(argstart);
        errno = ENOMEM;
        return -1;
    }
    
    if (!get_amiga_path(path_copy, 1024)) {
        /* No CLI available, try to execute directly */
        free(path_copy);
        free(argstart);
        return execve(filename, argstart, environ);
    }
    
    path = path_copy;
    
    /* No need for fnbuffer - we use test_path in the loop */
    
    /* Search each directory in PATH using Amiga path functions */
    path_ptr = path_copy;
    while (path_ptr != NULL) {
        char *test_path;
        BPTR lock;
        
        /* Find next colon or end of string */
        colon = strchr(path_ptr, ':');
        if (colon != NULL) {
            *colon = '\0';
        }
        
        /* Try to get a lock on the directory */
        if (strlen(path_ptr) > 0) {
            lock = Lock(path_ptr, SHARED_LOCK);
        } else {
            /* Empty path component means current directory */
            lock = GetCurrentDir();
        }
        
        if (lock) {
            /* Use AddPart to construct the full pathname */
            test_path = malloc(256);
            if (test_path) {
                if (strlen(path_ptr) > 0) {
                    strcpy(test_path, path_ptr);
                } else {
                    strcpy(test_path, "");
                }
                AddPart(test_path, filename, 256);
                
                /* Check if file exists and is executable */
                if (stat(test_path, &statbuf) == 0) {
                    if ((statbuf.st_mode & S_IFMT) == S_IFREG) {
                        /* Try to execute the file */
                        if (execve(test_path, argstart, environ) == 0) {
                            /* Success - this should not return */
                            free(argstart);
                            free(path_copy);
                            free(test_path);
                            UnLock(lock);
                            return 0;
                        }
                        
                        /* If ENOEXEC, try as shell script */
                        if (errno == ENOEXEC) {
                            /* Get shell from environment or use default */
                            shell = getenv("SHELL");
                            if (shell == NULL) {
                                shell = "C:Shell";
                            }
                            
                            /* Create new argument array with shell as first arg */
                            free(argstart);
                            argstart = malloc((argc + 2) * sizeof(char *));
                            if (argstart != NULL) {
                                argstart[0] = shell;
                                argstart[1] = test_path;
                                va_start(args, arg0);
                                for (i = 1; i < argc; i++) {
                                    argstart[i + 1] = va_arg(args, char *);
                                }
                                va_end(args);
                                argstart[argc + 1] = NULL;
                                
                                /* Try to execute as shell script */
                                execve(shell, argstart, environ);
                            }
                        }
                    }
                }
                free(test_path);
            }
            UnLock(lock);
        }
        
        /* Move to next path component */
        if (colon != NULL) {
            path_ptr = colon + 1;
        } else {
            path_ptr = NULL;
        }
    }
    
    /* All attempts failed */
    free(argstart);
    free(path_copy);
    errno = ENOENT;
    return -1;
}
