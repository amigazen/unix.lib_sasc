/*
 * SPDX-License-Identifier: BSD-2-Clause
 * strins.c - Insert string into another string
 *
 * Copyright (C) 2025 by amigazen project
 *
 * This function inserts one string into another string at the current position.
 * The destination string must have enough space allocated.
 */

#include <string.h>
#include "include/string.h"

/*
 * strins() - Insert string into another string
 * 
 * Inserts one string into another string at the current position.
 * The destination string must have enough space allocated.
 * This function modifies the destination string in place.
 *
 * Parameters:
 *   to - Destination string (must have sufficient space)
 *   fm - Source string to insert
 *
 * Returns:
 *   void (modifies destination string in place)
 */
void strins(char *to, char *fm)
{
    int tolen = strlen(to);
    int fmlen = strlen(fm);
    char *newto, *oldto;
    
    /* Move existing content to make room */
    newto = to + fmlen + tolen;
    oldto = to + tolen;
    tolen++;
    
    while (tolen--) {
        *newto-- = *oldto--;
    }
    
    /* Insert the new string */
    while (fmlen--) {
        *to++ = *fm++;
    }
}
