#ifndef _UNIX_MATH_H
#define _UNIX_MATH_H

#ifdef __SASC
#include "sc:include/math.h"
#else
#error Wrong compiler (SAS/C required)
#endif

#ifndef _STRICT_ANSI
double	copysign __ARGS((double, double));
int	finite __ARGS((double));
double	hypot __ARGS((double, double));
double	logb __ARGS((double));
double	scalb __ARGS((double, int));
#endif

/* C89-compatible macros for C99 math functions */
#ifndef isnan
#define isnan(x) ((x) != (x))
#endif

#ifndef isinf
#define isinf(x) ((x) == (1.0/0.0) || (x) == (-1.0/0.0))
#endif
#endif
