/*
 * SPDX-License-Identifier: BSD-2-Clause
 * gets.c - Deprecated string input function
 *
 * Based on PDC gets.c by J.A. Lydiatt
 * Copyright (C) 2025 by amigazen project
 *
 * WARNING: This function is DEPRECATED and UNSAFE!
 * Use fgets() or getline() instead for safe string input.
 */

#ifndef _WITH_SCLIB

/*
#pragma error "gets() function is DEPRECATED and UNSAFE! Use fgets() or getline() instead for more safe string input."
*/

#include <stdio.h>
#include "amiga.h"

/**
 * @brief Read a line from stdin (DEPRECATED - UNSAFE!)
 * @param s Buffer to store the line
 * @return Pointer to buffer on success, NULL on EOF or error
 * 
 * WARNING: This function is DEPRECATED and UNSAFE!
 * 
 * Security Issues:
 * - No buffer size checking - can cause buffer overflows
 * - No protection against malicious input
 * - Can crash programs or allow code execution
 * 
 * Use fgets() or getline() instead:
 * - fgets(buffer, size, stdin) - Safe with size limit
 * - getline(&line, &len, stdin) - POSIX safe alternative
 * 
 * This function is provided for C89 compatibility only.
 * DO NOT USE IN NEW CODE!
 */
char *gets(char *s)
{
    char *ptr;
    int c;
    
    if (s == NULL) {
        return NULL;
    }
    
    ptr = s;
    
    /* Read characters until newline or EOF */
    while ((c = getchar()) != EOF && c != '\n') {
        *ptr++ = (char)c;
        /* No bounds checking - this is the security vulnerability! */
    }
    
    /* Null terminate the string */
    *ptr = '\0';
    
    /* Return NULL if EOF and no characters read */
    if (c == EOF && ptr == s) {
        return NULL;
    }
    
    return s;
}

#else
/* Empty - functions provided by SAS/C sc.lib */
#endif
