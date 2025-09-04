#ifndef _UNIX_LOCALE_H
#define _UNIX_LOCALE_H

#include <stddef.h>  /* For size_t */

#ifdef __cplusplus
extern "C" {
#endif

/* Include SAS/C's locale functions */
#include "include:locale.h"  /* For setlocale, localeconv, etc. */

/* Additional POSIX locale functions if needed */
extern char *setlocale(int category, const char *locale);
extern struct lconv *localeconv(void);

#ifdef __cplusplus
}
#endif

#endif /* !_UNIX_LOCALE_H */
