/*
 * SPDX-License-Identifier: BSD-2-Clause
 * rindex.c - find last occurrence of a character in a string (BSD compatibility)
 *
 * Based on PDC rindex.c
 * Copyright (C) 2025 by amigazen project
 */

#include <string.h>

/**
 * @brief Find last occurrence of a character in a string
 * @param s String to search in
 * @param charwanted Character to search for
 * @return Pointer to last occurrence of charwanted in s, or NULL if not found
 * 
 * The rindex() function is a BSD compatibility function that finds the last
 * occurrence of the character 'charwanted' in the string 's'. It is equivalent
 * to strrchr() but provided for BSD compatibility.
 * 
 * This implementation simply calls strrchr() since they are functionally identical.
 */
char *rindex(const char *s, int charwanted)
{
    return strrchr(s, charwanted);
}