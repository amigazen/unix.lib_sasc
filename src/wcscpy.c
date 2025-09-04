/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * Copyright (c) 2025 amigazen project
 *
 * Wide character string copy function
 */

#include <wchar.h>

wchar_t *wcscpy(wchar_t *dest, const wchar_t *src)
{
    wchar_t *ptr;
    
    if (dest == NULL || src == NULL) {
        return dest;
    }
    
    ptr = dest;
    while (*src != WCHAR_NULL) {
        *ptr = *src;
        ptr++;
        src++;
    }
    *ptr = WCHAR_NULL;
    
    return dest;
}
