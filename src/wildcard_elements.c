/*
 * SPDX-License-Identifier: BSD-2-Clause
 * wildcard_elements.c - Wildcard element access and manipulation
 *
 * Copyright (C) 2025 by amigazen project
 *
 * This file provides functions for accessing and manipulating wildcard list elements.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/amiga.h"

/* Allocation step for dynamic string list growth */
#define ALLOC_STEP 50

/*
 * Get a string element from the list by index
 *
 * Parameters:
 *   index - Index of the element to retrieve
 *   strl  - Pointer to string list structure
 *
 * Returns:
 *   Pointer to the string, or NULL if index is invalid
 */
char *wildcard_pop_element(int index, wildcard_strlist *strl)
{
    char *ret = NULL;
    
    if (index >= 0 && index < strl->len) {
        ret = strl->string[index].str;
    }
    
    return ret;
}

/*
 * Add a string element to the list
 *
 * Parameters:
 *   elt  - String to add
 *   strl - Pointer to string list structure
 */
void wildcard_push_element(char *elt, wildcard_strlist *strl)
{
    /* Expand allocation if needed */
    if (strl->len >= strl->len_alloc) {
        strl->len_alloc += ALLOC_STEP;
        strl->string = realloc(strl->string, strl->len_alloc * sizeof(wildcard_str));
    }
    
    /* Add the string to the list */
    strl->string[strl->len].str = strdup(elt);
    strl->len++;
}

/*
 * Print all strings in the list to stdout
 *
 * Parameters:
 *   strl - Pointer to string list structure
 */
void wildcard_print_list(wildcard_strlist *strl)
{
    int i;
    for (i = 0; i < strl->len; i++) {
        puts(wildcard_pop_element(i, strl));
    }
    fflush(stdout);
}
