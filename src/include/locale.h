/* locale.h - ANSI C locale handling */

#ifndef _UNIX_LOCALE_H
#define _UNIX_LOCALE_H

#include <stddef.h>  /* For size_t */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __SASC
/* Include SAS/C's built-in locale.h */
#include "sc:include/locale.h"
#else
#ifndef LOCALE_H
#define LOCALE_H

/* ANSI C required types */
struct lconv {
    char *decimal_point;
    char *thousands_sep;
    char *grouping;
    char *int_curr_symbol;
    char *currency_symbol;
    char *mon_decimal_point;
    char *mon_thousands_sep;
    char *mon_grouping;
    char *positive_sign;
    char *negative_sign;
    char int_frac_digits;
    char frac_digits;
    char p_cs_precedes;
    char p_sep_by_space;
    char n_cs_precedes;
    char n_sep_by_space;
    char p_sign_posn;
    char n_sign_posn;
};

/* ANSI C required constants */
#define LC_ALL      0
#define LC_COLLATE  1
#define LC_CTYPE    2
#define LC_MONETARY 3
#define LC_NUMERIC  4
#define LC_TIME     5

/* ANSI C required functions */
char *setlocale(int category, const char *locale);
struct lconv *localeconv(void);

#endif /* LOCALE_H */
/* Additional POSIX locale functions if needed */
extern char *setlocale(int category, const char *locale);
extern struct lconv *localeconv(void);
#endif /* __SASC */

#ifdef __cplusplus
}
#endif

#endif /* !_UNIX_LOCALE_H */
