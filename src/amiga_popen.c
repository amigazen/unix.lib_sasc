/*
 * SPDX-License-Identifier: BSD-2-Clause
 * amiga_popen.c - Enhanced process execution for AmigaOS
 *
 * This module provides sophisticated popen/pclose functionality with
 * proper I/O redirection parsing and AmigaOS Execute() integration.
 * Based on the mypopen.c implementation from the Amiga Perl port.
 *
 * Copyright (C) 2025 by amigazen project
 */

#include "amiga.h"
#include "amiga_path_utils.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <proto/dos.h>

/* Constants for pipe handling */
#define AMIGA_PIPE_PATH "pipe:popenXXXXXXXX"
#define AMIGA_NULL_FILE "NULL:"
#define AMIGA_RUN_COMMAND "run >NULL: <NULL: "

/* Check if a command exists and is executable */
static int amiga_cmd_exists(const char *cmd)
{
    BPTR seg;
    
    if (!cmd) {
        return 0;
    }

    /* Try to load the command as a segment */
    seg = LoadSeg(cmd);
    if (seg) {
        UnLoadSeg(seg);
        return 1;
    }

    /* Try with .exe extension */
    {
        char *cmd_exe = malloc(strlen(cmd) + 5);
        if (cmd_exe) {
            strcpy(cmd_exe, cmd);
            strcat(cmd_exe, ".exe");
            seg = LoadSeg(cmd_exe);
            free(cmd_exe);
            if (seg) {
                UnLoadSeg(seg);
                return 1;
            }
        }
    }

    return 0;
}

/*
 * Parse command line for I/O redirection
 *
 * Parameters:
 *   cmd - Command string to parse
 *   in_file - Output: input file (NULL if none)
 *   out_file - Output: output file (NULL if none)
 *   args - Output: remaining arguments (NULL if none)
 *
 * Returns:
 *   0 on success, -1 on error
 */
static int amiga_parse_redirection(char *cmd, char **in_file, char **out_file, char **args)
{
    char *space_pos;
    char *in_pos, *out_pos;
    char *arg_pos;

    if (!cmd) {
        return -1;
    }

    *in_file = *out_file = *args = NULL;

    /* Find first space to separate command from arguments */
    space_pos = strchr(cmd, ' ');
    if (!space_pos) {
        return 0;  /* No arguments */
    }

    *space_pos = '\0';
    arg_pos = space_pos + 1;

    /* Parse redirection */
    in_pos = out_pos = NULL;

    while (*arg_pos) {
        /* Skip whitespace */
        while (*arg_pos == ' ') {
            arg_pos++;
        }
        if (!*arg_pos) {
            break;
        }

        if (*arg_pos == '<' && !in_pos) {
            in_pos = ++arg_pos;
            /* Find end of input file */
            while (*arg_pos && *arg_pos != ' ') {
                arg_pos++;
            }
            if (*arg_pos) {
                *arg_pos++ = '\0';
            }
        } else if (*arg_pos == '>' && !out_pos) {
            out_pos = ++arg_pos;
            /* Find end of output file */
            while (*arg_pos && *arg_pos != ' ') {
                arg_pos++;
            }
            if (*arg_pos) {
                *arg_pos++ = '\0';
            }
        } else {
            /* Regular argument */
            *args = arg_pos;
            break;
        }
    }

    *in_file = in_pos;
    *out_file = out_pos;
    return 0;
}

/*
 * Enhanced popen implementation for AmigaOS
 *
 * This function provides sophisticated process execution with:
 * - I/O redirection parsing
 * - Pipe handling for inter-process communication
 * - Proper AmigaOS Execute() integration
 * - Error handling and validation
 *
 * Parameters:
 *   cmd - Command string to execute
 *   mode - Open mode ("r" for read, "w" for write)
 *
 * Returns:
 *   FILE pointer on success, NULL on error
 */
FILE *amiga_popen(const char *cmd, const char *mode)
{
    char *cmd_copy, *real_cmd;
    char *in_file, *out_file, *args;
    char pipe_file[32];
    FILE *fp = NULL;
    struct FileHandle *null_handle;
    int exec_result;
    size_t cmd_len, real_cmd_len;

    if (!cmd || !mode) {
        errno = EINVAL;
        return NULL;
    }

    /* Skip leading spaces */
    while (*cmd == ' ') {
        cmd++;
    }

    /* Handle special case of "-" (stdin/stdout) */
    if (*cmd == '-' && *(cmd + 1) == '\0') {
        cmd = "perl";  /* Default to perl for now */
    }

    /* Allocate working buffers */
    cmd_len = strlen(cmd);
    real_cmd_len = cmd_len + strlen(AMIGA_RUN_COMMAND) + 256;  /* Extra space for redirection */
    
    cmd_copy = malloc(cmd_len + 1);
    real_cmd = malloc(real_cmd_len);
    if (!cmd_copy || !real_cmd) {
        free(cmd_copy);
        free(real_cmd);
        errno = ENOMEM;
        return NULL;
    }

    strcpy(cmd_copy, cmd);
    amigaizepath(cmd_copy);

    /* Parse command for I/O redirection */
    if (amiga_parse_redirection(cmd_copy, &in_file, &out_file, &args) < 0) {
        free(cmd_copy);
        free(real_cmd);
        errno = EINVAL;
        return NULL;
    }

    /* Check if command exists */
    if (!amiga_cmd_exists(cmd_copy)) {
        free(cmd_copy);
        free(real_cmd);
        errno = ENOENT;
        return NULL;
    }

    /* Generate unique pipe file name */
    strcpy(pipe_file, AMIGA_PIPE_PATH);
    if (!mktemp(pipe_file)) {
        free(cmd_copy);
        free(real_cmd);
        errno = EEXIST;
        return NULL;
    }

    /* Set up I/O based on mode */
    if (*mode == 'w') {
        /* Writing to process */
        if (in_file) {
            /* Input file specified, use NULL: for our output */
            fp = fopen(AMIGA_NULL_FILE, mode);
        } else {
            /* Use pipe for input to process */
            in_file = pipe_file;
        }
        if (!out_file) {
            out_file = AMIGA_NULL_FILE;
        }
    } else {
        /* Reading from process */
        if (out_file) {
            /* Output file specified, use NULL: for our input */
            fp = fopen(AMIGA_NULL_FILE, mode);
        } else {
            /* Use pipe for output from process */
            out_file = pipe_file;
            fp = fopen(pipe_file, mode);
        }
        if (!in_file) {
            in_file = AMIGA_NULL_FILE;
        }
    }

    /* Build execution command */
    strcpy(real_cmd, AMIGA_RUN_COMMAND);
    strcat(real_cmd, cmd_copy);

    if (out_file) {
        strcat(real_cmd, " >");
        strcat(real_cmd, out_file);
    }
    if (in_file) {
        strcat(real_cmd, " <");
        strcat(real_cmd, in_file);
    }
    if (args) {
        strcat(real_cmd, " ");
        strcat(real_cmd, args);
    }

    /* Execute the command */
    null_handle = (struct FileHandle *)Open(AMIGA_NULL_FILE, MODE_OLDFILE);
    exec_result = Execute(real_cmd, null_handle, null_handle);
    Delay(20);  /* Give process time to start */
    Close((BPTR)null_handle);

    if (!exec_result) {
        /* Execution failed */
        if (fp && fp != stdin && fp != stdout && fp != stderr) {
            fclose(fp);
        }
        free(cmd_copy);
        free(real_cmd);
        errno = EOSERR;
        return NULL;
    }

    /* Return appropriate file handle */
    if (!fp) {
        fp = fopen(pipe_file, mode);
    }

    free(cmd_copy);
    free(real_cmd);
    return fp;
}

/*
 * Enhanced pclose implementation for AmigaOS
 *
 * Parameters:
 *   stream - FILE stream to close
 *
 * Returns:
 *   0 on success, -1 on error
 */
int amiga_pclose(FILE *stream)
{
    if (!stream) {
        errno = EINVAL;
        return -1;
    }

    /* Simply close the file stream */
    /* Unlike Unix, we don't need to wait for the process */
    return fclose(stream);
}

/*
 * Check if a command exists in the system
 *
 * Parameters:
 *   cmd - Command name to check
 *
 * Returns:
 *   1 if command exists, 0 otherwise
 */
int amiga_cmd_exists_public(const char *cmd)
{
    return amiga_cmd_exists(cmd);
}
