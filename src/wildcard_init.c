/*
 * SPDX-License-Identifier: BSD-2-Clause
 * wildcard_init.c - Wildcard list initialization and management
 *
 * Copyright (C) 2025 by amigazen project
 *
 * This file provides functions for initializing and managing wildcard string lists.
 */

#include <stdlib.h>
#include "include/amiga.h"

/*
 * Initialize a string list structure
 *
 * Parameters:
 *   strl - Pointer to string list structure to initialize
 */
void wildcard_init_list(wildcard_strlist *strl)
{
    strl->string = NULL;
    strl->len = 0;
    strl->len_alloc = 0;
}

/*
 * Clear and free all memory associated with a string list
 *
 * Parameters:
 *   strl - Pointer to string list structure to clear
 */
void wildcard_clear_list(wildcard_strlist *strl)
{
    int i;
    
    /* Free all individual strings */
    for (i = 0; i < strl->len; i++) {
        free(wildcard_pop_element(i, strl));
    }
    
    /* Reset list state */
    strl->len = 0;
    if (strl->string != NULL) {
        free(strl->string);
    }
    strl->string = NULL;
}

/*
 * Get the number of elements in the string list
 *
 * Parameters:
 *   strl - Pointer to string list structure
 *
 * Returns:
 *   Number of elements in the list
 */
int wildcard_get_count(wildcard_strlist *strl)
{
    return strl->len;
}

/*
 * Check if a string list is empty
 *
 * Parameters:
 *   strl - Pointer to string list structure
 *
 * Returns:
 *   1 if empty, 0 if not empty
 */
int wildcard_is_empty(wildcard_strlist *strl)
{
    return (strl->len == 0) ? 1 : 0;
}
