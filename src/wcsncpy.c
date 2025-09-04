/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * Copyright (c) 2025 amigazen project
 *
 * Wide character string copy with length limit
 */

#include <wchar.h>

wchar_t *wcsncpy(wchar_t *dest, const wchar_t *src, size_t n)
{
    wchar_t *ptr;
    size_t i;
    
    if (dest == NULL || src == NULL) {
        return dest;
    }
    
    ptr = dest;
    i = 0;
    
    /* Copy characters up to n or until null terminator */
    while (i < n && *src != WCHAR_NULL) {
        *ptr = *src;
        ptr++;
        src++;
        i++;
    }
    
    /* Pad with null characters if needed */
    while (i < n) {
        *ptr = WCHAR_NULL;
        ptr++;
        i++;
    }
    
    return dest;
}
