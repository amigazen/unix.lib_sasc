/*
 * SPDX-License-Identifier: BSD-2-Clause
 * atol.c - convert string to long integer
 *
 * Based on PDC stdlib atol.c by J.A. Lydiatt
 * Copyright (C) 2025 by amigazen project
 */

#include <stdlib.h>
#include <stddef.h>
#include "amiga.h"
#include <proto/dos.h>

long atol(const char *number)
{
    long value;
    long chars;
    
    if (number == NULL) {
        return 0;
    }
    
    /* Use Amiga native StrToLong for decimal conversion */
    chars = StrToLong((STRPTR)number, &value);
    
    /* StrToLong returns -1 if no digits found, 0 if successful */
    if (chars == -1) {
        return 0;
    }
    
    return value;
}

int atoi(const char *number)
{
    return (int)atol(number);
}
