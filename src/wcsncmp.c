/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * Copyright (c) 2025 amigazen project
 *
 * Wide character string comparison with length limit
 */

#include <wchar.h>

int wcsncmp(const wchar_t *s1, const wchar_t *s2, size_t n)
{
    size_t i;
    
    if (s1 == NULL && s2 == NULL) {
        return 0;
    }
    if (s1 == NULL) {
        return -1;
    }
    if (s2 == NULL) {
        return 1;
    }
    
    i = 0;
    while (i < n && *s1 == *s2 && *s1 != WCHAR_NULL && *s2 != WCHAR_NULL) {
        s1++;
        s2++;
        i++;
    }
    
    if (i >= n || *s1 == *s2) {
        return 0;
    } else if (*s1 < *s2) {
        return -1;
    } else {
        return 1;
    }
}
