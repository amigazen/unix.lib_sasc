#ifndef _UNIX_TGMATH_H
#define _UNIX_TGMATH_H

#include <math.h>
#include <complex.h>

/* Type-generic math macros */
#define acos(x)     _Generic((x), \
    float: acosf, \
    double: acos, \
    long double: acosl, \
    default: acos)(x)

#define asin(x)     _Generic((x), \
    float: asinf, \
    double: asin, \
    long double: asinl, \
    default: asin)(x)

#define atan(x)     _Generic((x), \
    float: atanf, \
    double: atan, \
    long double: atanl, \
    default: atan)(x)

#define cos(x)      _Generic((x), \
    float: cosf, \
    double: cos, \
    long double: cosl, \
    default: cos)(x)

#define sin(x)      _Generic((x), \
    float: sinf, \
    double: sin, \
    long double: sinl, \
    default: sin)(x)

#define tan(x)      _Generic((x), \
    float: tanf, \
    double: tan, \
    long double: tanl, \
    default: tan)(x)

#define exp(x)      _Generic((x), \
    float: expf, \
    double: exp, \
    long double: expl, \
    default: exp)(x)

#define log(x)      _Generic((x), \
    float: logf, \
    double: log, \
    long double: logl, \
    default: log)(x)

#define sqrt(x)     _Generic((x), \
    float: sqrtf, \
    double: sqrt, \
    long double: sqrtl, \
    default: sqrt)(x)

#define pow(x, y)   _Generic((x), \
    float: powf, \
    double: pow, \
    long double: powl, \
    default: pow)(x, y)

#endif /* !_UNIX_TGMATH_H */
