/*
 * SPDX-License-Identifier: BSD-2-Clause
 * index.c - find first occurrence of a character in a string (BSD compatibility)
 *
 * Based on PDC index.c
 * Copyright (C) 2025 by amigazen project
 */

#include <string.h>

/**
 * @brief Find first occurrence of a character in a string
 * @param s String to search in
 * @param charwanted Character to search for
 * @return Pointer to first occurrence of charwanted in s, or NULL if not found
 * 
 * The index() function is a BSD compatibility function that finds the first
 * occurrence of the character 'charwanted' in the string 's'. It is equivalent
 * to strchr() but provided for BSD compatibility.
 * 
 * This implementation simply calls strchr() since they are functionally identical.
 */
char *index(const char *s, int charwanted)
{
    return strchr(s, charwanted);
}