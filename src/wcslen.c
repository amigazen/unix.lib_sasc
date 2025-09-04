/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * Copyright (c) 2025 amigazen project
 *
 * Wide character string length function
 */

#include <wchar.h>

size_t wcslen(const wchar_t *s)
{
    size_t n = 0;
    
    if (s == NULL) {
        return 0;
    }
    
    while (s[n] != WCHAR_NULL) {
        n++;
    }
    
    return n;
}
