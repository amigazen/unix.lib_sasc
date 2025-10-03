/* limits.h - ANSI C numerical limits */

#ifdef __SASC
/* Include SAS/C's built-in limits.h */
#include "sc:include/limits.h"
#else
#ifndef LIMITS_H
#define LIMITS_H

#include <stddef.h>

/* Number of bits in a byte */
#define CHAR_BIT 8

/* Minimum and maximum values for signed char */
#define SCHAR_MIN (-128)
#define SCHAR_MAX 127

/* Maximum value for unsigned char */
#define UCHAR_MAX 255

/* Minimum and maximum values for char */
#define CHAR_MIN SCHAR_MIN
#define CHAR_MAX SCHAR_MAX

/* Maximum number of bytes in a multibyte character */
#define MB_LEN_MAX 1

/* Minimum and maximum values for short int */
#define SHRT_MIN (-32768)
#define SHRT_MAX 32767

/* Maximum value for unsigned short int */
#define USHRT_MAX 65535

/* Minimum and maximum values for int */
#define INT_MIN (-2147483647 - 1)
#define INT_MAX 2147483647

/* Maximum value for unsigned int */
#define UINT_MAX 4294967295U

/* Minimum and maximum values for long int */
#define LONG_MIN (-2147483647L - 1L)
#define LONG_MAX 2147483647L

/* Maximum value for unsigned long int */
#define ULONG_MAX 4294967295UL

/* C99 long long limits */
#define LLONG_MIN (-9223372036854775807LL - 1LL)
#define LLONG_MAX 9223372036854775807LL
#define ULLONG_MAX 18446744073709551615ULL

/* Non-ANSI extensions */
#define BITSPERBYTE CHAR_BIT
#define PATHSIZE 1024
#define MAXPATH PATHSIZE
#define MAXSTRING 1024

#endif /* LIMITS_H */

#endif /* __SASC */
