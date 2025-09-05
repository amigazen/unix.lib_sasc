#ifndef _UNIX_WCHAR_H
#define _UNIX_WCHAR_H

#include <stddef.h>  /* For size_t */

#ifdef __cplusplus
extern "C" {
#endif

/* Use SAS/C's wchar_t definition - it's defined as char */
#ifndef _WCHAR_T
#define _WCHAR_T
typedef char wchar_t;
#endif

/* Wide character null terminator */
#ifndef WCHAR_NULL
#define WCHAR_NULL ((wchar_t)0)
#endif

#ifdef __SASC
/* Include SAS/C's multibyte functions */
#include "sc:include/stdlib.h"  /* For mblen, mbtowc, wctomb, mbstowcs, wcstombs */
#else
#error Wrong compiler (SAS/C required)
#endif

/* Wide character string functions */
extern size_t wcslen(const wchar_t *s);
extern wchar_t *wcscpy(wchar_t *dest, const wchar_t *src);
extern wchar_t *wcsncpy(wchar_t *dest, const wchar_t *src, size_t n);
extern wchar_t *wcscat(wchar_t *dest, const wchar_t *src);
extern int wcscmp(const wchar_t *s1, const wchar_t *s2);
extern int wcsncmp(const wchar_t *s1, const wchar_t *s2, size_t n);

#ifdef __cplusplus
}
#endif

#endif /* !_UNIX_WCHAR_H */
