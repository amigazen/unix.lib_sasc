/*
 * SPDX-License-Identifier: BSD-2-Clause
 * toint.c - convert character to integer value
 *
 * Based on PDC stdlib toint function by J.A. Lydiatt
 * Copyright (C) 2025 by amigazen project
 */

#include <ctype.h>
int toint(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    else if (c >= 'A' && c <= 'Z')
        return c - 'A' + 10;
    else if (c >= 'a' && c <= 'z')
        return c - 'a' + 10;
    else
        return -1;
}
