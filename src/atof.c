/*
 * SPDX-License-Identifier: BSD-2-Clause
 * atof.c - convert string to double
 *
 * Based on PDC stdlib atof.c by J.A. Lydiatt
 * Copyright (C) 2025 by amigazen project
 */

#ifndef _WITH_SCLIB

#include <stdlib.h>
#include <stddef.h>

double atof(const char *str)
{
    return strtod(str, (char **)NULL);
}

#else
/* Empty - function provided by SAS/C sc.lib */
#endif
