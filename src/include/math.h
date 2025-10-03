/* math.h - ANSI C mathematical functions and constants */

#ifndef _UNIX_MATH_H
#define _UNIX_MATH_H

#ifdef __SASC
/* Include SAS/C's built-in math.h */
#include "sc:include/math.h"
#else

#ifndef MATH_H
#define MATH_H

#include <stddef.h>

/* ANSI C required mathematical constants */
#ifndef HUGE_VAL
#define HUGE_VAL (1.7976931348623157e+308)
#endif

/* ANSI C required mathematical functions */
double acos(double x);
double asin(double x);
double atan(double x);
double atan2(double y, double x);
double cos(double x);
double sin(double x);
double tan(double x);

double cosh(double x);
double sinh(double x);
double tanh(double x);

double exp(double x);
double frexp(double value, int *exp);
double ldexp(double x, int exp);
double log(double x);
double log10(double x);
double modf(double value, double *iptr);
double pow(double x, double y);
double sqrt(double x);

double ceil(double x);
double fabs(double x);
double floor(double x);
double fmod(double x, double y);

/* Non-ANSI extensions for compatibility */
double acosh(double x);
double asinh(double x);
double atanh(double x);
double cabs(double x);
double cbrt(double x);
double copysign(double x, double y);
double drem(double x, double y);
double expm1(double x);
double hypot(double x, double y);
double log1p(double x);
double logb(double x);
double rint(double x);
double scalb(double x, double n);

/* Mathematical constants */
#ifndef MAXDOUBLE
#define BITS(type) (8 * (int)sizeof(type))

#define MAXDOUBLE 1.79769313486231470e+308
#define MAXFLOAT ((float)3.40282346638528860e+38)
#define MINDOUBLE 4.94065645841246544e-324
#define MINFLOAT ((float)1.40129846432481707e-45)

#define DMINEXP (-(DMAXEXP + DSIGNIF - 4))
#define FMINEXP (-(FMAXEXP + FSIGNIF - 4))

#define DSIGNIF (BITS(double)-11)
#define FSIGNIF (BITS(float)-8)

#define DMAXEXP (1 << 11 - 1)
#define FMAXEXP (1 << 8 - 1)
#endif

#endif /* MATH_H */

#endif /* __SASC */

/* C99 math classification macros */
#ifndef fpclassify
#define fpclassify(x) ((x) != (x) ? FP_NAN : \
                      (x) == (1.0/0.0) || (x) == (-1.0/0.0) ? FP_INFINITE : \
                      (x) == 0.0 ? FP_ZERO : \
                      (x) < 0.0 ? FP_NORMAL : FP_NORMAL)
#endif

#ifndef isfinite
#define isfinite(x) ((x) == (x) && (x) != (1.0/0.0) && (x) != (-1.0/0.0))
#endif

#ifndef isinf
#define isinf(x) ((x) == (1.0/0.0) || (x) == (-1.0/0.0))
#endif

#ifndef isnan
#define isnan(x) ((x) != (x))
#endif

#ifndef isnormal
#define isnormal(x) (isfinite(x) && (x) != 0.0)
#endif

#ifndef signbit
#define signbit(x) ((x) < 0.0)
#endif

/* C99 math classification constants */
#define FP_NAN       0
#define FP_INFINITE  1
#define FP_ZERO      2
#define FP_NORMAL    3
#define FP_SUBNORMAL 4

/* C99 additional math functions */
double fma(double x, double y, double z);
double fmax(double x, double y);
double fmin(double x, double y);
double trunc(double x);
double round(double x);
double nearbyint(double x);
double rint(double x);
long int lrint(double x);
long long int llrint(double x);
long int lround(double x);
long long int llround(double x);

/* C89-compatible macros for C99 math functions */
#ifndef isnan
#define isnan(x) ((x) != (x))
#endif

#ifndef isinf
#define isinf(x) ((x) == (1.0/0.0) || (x) == (-1.0/0.0))
#endif

#endif /* !_UNIX_MATH_H */
