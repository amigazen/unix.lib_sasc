/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * Copyright (c) 2025 amigazen project
 *
 * Wide character string concatenation function
 */

#include <wchar.h>

wchar_t *wcscat(wchar_t *dest, const wchar_t *src)
{
    wchar_t *ptr;
    
    if (dest == NULL || src == NULL) {
        return dest;
    }
    
    /* Find end of destination string */
    ptr = dest;
    while (*ptr != WCHAR_NULL) {
        ptr++;
    }
    
    /* Copy source string to end of destination */
    while (*src != WCHAR_NULL) {
        *ptr = *src;
        ptr++;
        src++;
    }
    *ptr = WCHAR_NULL;
    
    return dest;
}
