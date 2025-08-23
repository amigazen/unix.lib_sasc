#ifndef _UNIX_COMPLEX_H
#define _UNIX_COMPLEX_H

#ifdef __cplusplus
extern "C" {
#endif

/* Complex number type */
typedef struct {
    double real;
    double imag;
} complex;

/* Complex number constants */
#define complex_I ((complex){0.0, 1.0})

/* Complex number creation */
#define creal(z) ((z).real)
#define cimag(z) ((z).imag)
#define cabs(z) (sqrt((z).real * (z).real + (z).imag * (z).imag))

/* Complex arithmetic functions */
complex cadd(complex a, complex b);
complex csub(complex a, complex b);
complex cmul(complex a, complex b);
complex cdiv(complex a, complex b);

/* Complex exponential and logarithmic functions */
complex cexp(complex z);
complex clog(complex z);
complex cpow(complex z, complex w);

/* Complex trigonometric functions */
complex csin(complex z);
complex ccos(complex z);
complex ctan(complex z);
complex casin(complex z);
complex cacos(complex z);
complex catan(complex z);

/* Complex hyperbolic functions */
complex csinh(complex z);
complex ccosh(complex z);
complex ctanh(complex z);
complex casinh(complex z);
complex cacosh(complex z);
complex catanh(complex z);

#ifdef __cplusplus
}
#endif

#endif /* !_UNIX_COMPLEX_H */
