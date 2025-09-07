/*
 * terminfo_parameterized.c - Parameterized string functions
 *
 * Implements tparm and tiparm functions for handling parameterized
 * terminal control strings with dynamic parameter substitution.
 *
 * Copyright (c) 2025 amigazen project
 * 
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "amiga_termcap_private.h"
#include "include/internal/amiga_terminfo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

/* Static buffer for tparm results */
static char tparm_buffer[1024];

/* Internal tparm implementation with up to 9 parameters */
char *amiga_terminfo_tparm_internal(const char *str, long p1, long p2, long p3, 
                                   long p4, long p5, long p6, long p7, long p8, long p9)
{
    const char *src;
    char *dst;
    int param_count;
    long params[9];
    int i;
    
    if (!str) return NULL;
    
    /* Initialize parameters */
    params[0] = p1;
    params[1] = p2;
    params[2] = p3;
    params[3] = p4;
    params[4] = p5;
    params[5] = p6;
    params[6] = p7;
    params[7] = p8;
    params[8] = p9;
    
    /* Count non-zero parameters */
    param_count = 0;
    for (i = 0; i < 9; i++) {
        if (params[i] != 0) param_count = i + 1;
    }
    
    src = str;
    dst = tparm_buffer;
    
    while (*src && (dst - tparm_buffer) < sizeof(tparm_buffer) - 1) {
        if (*src == '%') {
            src++;
            if (*src == '%') {
                /* Literal % */
                *dst++ = '%';
                src++;
            } else if (*src >= '1' && *src <= '9') {
                /* Parameter substitution */
                int param_num;
                int len;
                
                param_num = *src - '1';
                if (param_num < param_count) {
                    len = sprintf(dst, "%ld", params[param_num]);
                    dst += len;
                }
                src++;
            } else if (*src == 'd') {
                /* First parameter as decimal */
                int len;
                
                if (param_count > 0) {
                    len = sprintf(dst, "%ld", params[0]);
                    dst += len;
                }
                src++;
            } else if (*src == 'i') {
                /* Increment parameters */
                int len;
                
                if (param_count > 0) {
                    len = sprintf(dst, "%ld", params[0] + 1);
                    dst += len;
                }
                src++;
            } else if (*src == '2') {
                /* Two-digit parameter */
                int len;
                
                if (param_count > 0) {
                    len = sprintf(dst, "%02ld", params[0]);
                    dst += len;
                }
                src++;
            } else if (*src == '3') {
                /* Three-digit parameter */
                int len;
                
                if (param_count > 0) {
                    len = sprintf(dst, "%03ld", params[0]);
                    dst += len;
                }
                src++;
            } else if (*src == 'c') {
                /* Character parameter */
                if (param_count > 0) {
                    *dst++ = (char)(params[0] & 0xFF);
                }
                src++;
            } else if (*src == 's') {
                /* String parameter (not supported in basic implementation) */
                src++;
            } else if (*src == 'x') {
                /* Hexadecimal parameter */
                int len;
                
                if (param_count > 0) {
                    len = sprintf(dst, "%lx", params[0]);
                    dst += len;
                }
                src++;
            } else if (*src == 'X') {
                /* Uppercase hexadecimal parameter */
                int len;
                
                if (param_count > 0) {
                    len = sprintf(dst, "%lX", params[0]);
                    dst += len;
                }
                src++;
            } else if (*src == 'o') {
                /* Octal parameter */
                int len;
                
                if (param_count > 0) {
                    len = sprintf(dst, "%lo", params[0]);
                    dst += len;
                }
                src++;
            } else if (*src == 'p') {
                /* Push parameter (not supported in basic implementation) */
                src++;
            } else if (*src == 'P') {
                /* Pop parameter (not supported in basic implementation) */
                src++;
            } else if (*src == 'g') {
                /* Get parameter (not supported in basic implementation) */
                src++;
            } else if (*src == 'G') {
                /* Set parameter (not supported in basic implementation) */
                src++;
            } else if (*src == 'r') {
                /* Reverse parameters (not supported in basic implementation) */
                src++;
            } else if (*src == 'R') {
                /* Rotate parameters (not supported in basic implementation) */
                src++;
            } else if (*src == 't') {
                /* Conditional parameter (not supported in basic implementation) */
                src++;
            } else if (*src == 'T') {
                /* Conditional parameter (not supported in basic implementation) */
                src++;
            } else if (*src == 'l') {
                /* Length of string (not supported in basic implementation) */
                src++;
            } else if (*src == 'L') {
                /* Length of string (not supported in basic implementation) */
                src++;
            } else if (*src == 'n') {
                /* Negate parameter */
                int len;
                
                if (param_count > 0) {
                    len = sprintf(dst, "%ld", -params[0]);
                    dst += len;
                }
                src++;
            } else if (*src == 'm') {
                /* Modulo parameter */
                int len;
                
                if (param_count > 1) {
                    len = sprintf(dst, "%ld", params[0] % params[1]);
                    dst += len;
                }
                src++;
            } else if (*src == 'a') {
                /* Add parameters */
                int len;
                
                if (param_count > 1) {
                    len = sprintf(dst, "%ld", params[0] + params[1]);
                    dst += len;
                }
                src++;
            } else if (*src == 'A') {
                /* Add parameters */
                int len;
                
                if (param_count > 1) {
                    len = sprintf(dst, "%ld", params[0] + params[1]);
                    dst += len;
                }
                src++;
            } else if (*src == 's') {
                /* Subtract parameters */
                int len;
                
                if (param_count > 1) {
                    len = sprintf(dst, "%ld", params[0] - params[1]);
                    dst += len;
                }
                src++;
            } else if (*src == 'S') {
                /* Subtract parameters */
                int len;
                
                if (param_count > 1) {
                    len = sprintf(dst, "%ld", params[0] - params[1]);
                    dst += len;
                }
                src++;
            } else if (*src == 'm') {
                /* Multiply parameters */
                int len;
                
                if (param_count > 1) {
                    len = sprintf(dst, "%ld", params[0] * params[1]);
                    dst += len;
                }
                src++;
            } else if (*src == 'M') {
                /* Multiply parameters */
                int len;
                
                if (param_count > 1) {
                    len = sprintf(dst, "%ld", params[0] * params[1]);
                    dst += len;
                }
                src++;
            } else if (*src == 'd') {
                /* Divide parameters */
                int len;
                
                if (param_count > 1 && params[1] != 0) {
                    len = sprintf(dst, "%ld", params[0] / params[1]);
                    dst += len;
                }
                src++;
            } else if (*src == 'D') {
                /* Divide parameters */
                int len;
                
                if (param_count > 1 && params[1] != 0) {
                    len = sprintf(dst, "%ld", params[0] / params[1]);
                    dst += len;
                }
                src++;
            } else if (*src == 'e') {
                /* Equal parameters */
                int len;
                
                if (param_count > 1) {
                    len = sprintf(dst, "%ld", (params[0] == params[1]) ? 1 : 0);
                    dst += len;
                }
                src++;
            } else if (*src == 'E') {
                /* Equal parameters */
                int len;
                
                if (param_count > 1) {
                    len = sprintf(dst, "%ld", (params[0] == params[1]) ? 1 : 0);
                    dst += len;
                }
                src++;
            } else if (*src == 'g') {
                /* Greater than parameters */
                int len;
                
                if (param_count > 1) {
                    len = sprintf(dst, "%ld", (params[0] > params[1]) ? 1 : 0);
                    dst += len;
                }
                src++;
            } else if (*src == 'G') {
                /* Greater than parameters */
                int len;
                
                if (param_count > 1) {
                    len = sprintf(dst, "%ld", (params[0] > params[1]) ? 1 : 0);
                    dst += len;
                }
                src++;
            } else if (*src == 'l') {
                /* Less than parameters */
                int len;
                
                if (param_count > 1) {
                    len = sprintf(dst, "%ld", (params[0] < params[1]) ? 1 : 0);
                    dst += len;
                }
                src++;
            } else if (*src == 'L') {
                /* Less than parameters */
                int len;
                
                if (param_count > 1) {
                    len = sprintf(dst, "%ld", (params[0] < params[1]) ? 1 : 0);
                    dst += len;
                }
                src++;
            } else if (*src == 'n') {
                /* Not equal parameters */
                int len;
                
                if (param_count > 1) {
                    len = sprintf(dst, "%ld", (params[0] != params[1]) ? 1 : 0);
                    dst += len;
                }
                src++;
            } else if (*src == 'N') {
                /* Not equal parameters */
                int len;
                
                if (param_count > 1) {
                    len = sprintf(dst, "%ld", (params[0] != params[1]) ? 1 : 0);
                    dst += len;
                }
                src++;
            } else if (*src == 'o') {
                /* Or parameters */
                int len;
                
                if (param_count > 1) {
                    len = sprintf(dst, "%ld", params[0] | params[1]);
                    dst += len;
                }
                src++;
            } else if (*src == 'O') {
                /* Or parameters */
                int len;
                
                if (param_count > 1) {
                    len = sprintf(dst, "%ld", params[0] | params[1]);
                    dst += len;
                }
                src++;
            } else if (*src == 'a') {
                /* And parameters */
                int len;
                
                if (param_count > 1) {
                    len = sprintf(dst, "%ld", params[0] & params[1]);
                    dst += len;
                }
                src++;
            } else if (*src == 'A') {
                /* And parameters */
                int len;
                
                if (param_count > 1) {
                    len = sprintf(dst, "%ld", params[0] & params[1]);
                    dst += len;
                }
                src++;
            } else if (*src == 'x') {
                /* Xor parameters */
                int len;
                
                if (param_count > 1) {
                    len = sprintf(dst, "%ld", params[0] ^ params[1]);
                    dst += len;
                }
                src++;
            } else if (*src == 'X') {
                /* Xor parameters */
                int len;
                
                if (param_count > 1) {
                    len = sprintf(dst, "%ld", params[0] ^ params[1]);
                    dst += len;
                }
                src++;
            } else if (*src == '!') {
                /* Not parameter */
                int len;
                
                if (param_count > 0) {
                    len = sprintf(dst, "%ld", !params[0]);
                    dst += len;
                }
                src++;
            } else if (*src == '~') {
                /* Bitwise not parameter */
                int len;
                
                if (param_count > 0) {
                    len = sprintf(dst, "%ld", ~params[0]);
                    dst += len;
                }
                src++;
            } else if (*src == '+') {
                /* Add 1 to parameter */
                int len;
                
                if (param_count > 0) {
                    len = sprintf(dst, "%ld", params[0] + 1);
                    dst += len;
                }
                src++;
            } else if (*src == '-') {
                /* Subtract 1 from parameter */
                int len;
                
                if (param_count > 0) {
                    len = sprintf(dst, "%ld", params[0] - 1);
                    dst += len;
                }
                src++;
            } else if (*src == '*') {
                /* Multiply parameter by 2 */
                int len;
                
                if (param_count > 0) {
                    len = sprintf(dst, "%ld", params[0] * 2);
                    dst += len;
                }
                src++;
            } else if (*src == '/') {
                /* Divide parameter by 2 */
                int len;
                
                if (param_count > 0) {
                    len = sprintf(dst, "%ld", params[0] / 2);
                    dst += len;
                }
                src++;
            } else if (*src == '^') {
                /* Power of 2 */
                long result;
                int i;
                int len;
                
                if (param_count > 0) {
                    result = 1;
                    for (i = 0; i < params[0]; i++) {
                        result *= 2;
                    }
                    len = sprintf(dst, "%ld", result);
                    dst += len;
                }
                src++;
            } else if (*src == 'v') {
                /* Variable parameter (not supported in basic implementation) */
                src++;
            } else if (*src == 'V') {
                /* Variable parameter (not supported in basic implementation) */
                src++;
            } else if (*src == 'w') {
                /* Width parameter (not supported in basic implementation) */
                src++;
            } else if (*src == 'W') {
                /* Width parameter (not supported in basic implementation) */
                src++;
            } else if (*src == 'h') {
                /* Height parameter (not supported in basic implementation) */
                src++;
            } else if (*src == 'H') {
                /* Height parameter (not supported in basic implementation) */
                src++;
            } else if (*src == 'z') {
                /* Zero parameter (not supported in basic implementation) */
                src++;
            } else if (*src == 'Z') {
                /* Zero parameter (not supported in basic implementation) */
                src++;
            } else if (*src == 'q') {
                /* Quote parameter (not supported in basic implementation) */
                src++;
            } else if (*src == 'Q') {
                /* Quote parameter (not supported in basic implementation) */
                src++;
            } else if (*src == 'u') {
                /* Unquote parameter (not supported in basic implementation) */
                src++;
            } else if (*src == 'U') {
                /* Unquote parameter (not supported in basic implementation) */
                src++;
            } else if (*src == 'j') {
                /* Jump parameter (not supported in basic implementation) */
                src++;
            } else if (*src == 'J') {
                /* Jump parameter (not supported in basic implementation) */
                src++;
            } else if (*src == 'k') {
                /* Key parameter (not supported in basic implementation) */
                src++;
            } else if (*src == 'K') {
                /* Key parameter (not supported in basic implementation) */
                src++;
            } else if (*src == 'y') {
                /* Year parameter (not supported in basic implementation) */
                src++;
            } else if (*src == 'Y') {
                /* Year parameter (not supported in basic implementation) */
                src++;
            } else if (*src == 'z') {
                /* Zone parameter (not supported in basic implementation) */
                src++;
            } else if (*src == 'Z') {
                /* Zone parameter (not supported in basic implementation) */
                src++;
            } else {
                /* Unknown format specifier, copy as-is */
                *dst++ = '%';
                *dst++ = *src++;
            }
        } else {
            /* Regular character */
            *dst++ = *src++;
        }
    }
    
    *dst = '\0';
    return tparm_buffer;
}

/* Public API functions */
char *tparm(const char *str, ...)
{
    va_list args;
    long p1, p2, p3, p4, p5, p6, p7, p8, p9;
    
    if (!str) return NULL;
    
    va_start(args, str);
    p1 = va_arg(args, long);
    p2 = va_arg(args, long);
    p3 = va_arg(args, long);
    p4 = va_arg(args, long);
    p5 = va_arg(args, long);
    p6 = va_arg(args, long);
    p7 = va_arg(args, long);
    p8 = va_arg(args, long);
    p9 = va_arg(args, long);
    va_end(args);
    
    return amiga_terminfo_tparm_internal(str, p1, p2, p3, p4, p5, p6, p7, p8, p9);
}

char *tiparm(const char *str, long p1, long p2, long p3, long p4, long p5, 
             long p6, long p7, long p8, long p9)
{
    return amiga_terminfo_tparm_internal(str, p1, p2, p3, p4, p5, p6, p7, p8, p9);
}

int tputs(const char *str, int affcnt, int (*putc)(int))
{
    if (!str || !putc) return -1;
    
    /* Output the string character by character */
    while (*str) {
        if (putc(*str) == EOF) return -1;
        str++;
    }
    
    return 0;
}

int putp(const char *str)
{
    if (!str) return -1;
    
    /* Output the string to stdout */
    printf("%s", str);
    fflush(stdout);
    
    return 0;
}
