#ifndef _UNIX_FENV_H
#define _UNIX_FENV_H

#ifdef __cplusplus
extern "C" {
#endif

/* Floating-point environment types */
typedef unsigned int fenv_t;
typedef unsigned int fexcept_t;

/* Floating-point exception flags */
#define FE_DIVBYZERO    0x01
#define FE_INEXACT      0x02
#define FE_INVALID      0x04
#define FE_OVERFLOW     0x08
#define FE_UNDERFLOW    0x10
#define FE_ALL_EXCEPT   (FE_DIVBYZERO | FE_INEXACT | FE_INVALID | FE_OVERFLOW | FE_UNDERFLOW)

/* Rounding direction macros */
#define FE_DOWNWARD     0x01
#define FE_TONEAREST    0x02
#define FE_TOWARDZERO   0x04
#define FE_UPWARD       0x08

/* Default environment */
#define FE_DFL_ENV      ((const fenv_t *) 0)

/* Floating-point environment functions */
int feclearexcept(int excepts);
int fegetexceptflag(fexcept_t *flagp, int excepts);
int feraiseexcept(int excepts);
int fesetexceptflag(const fexcept_t *flagp, int excepts);
int fetestexcept(int excepts);

int fegetround(void);
int fesetround(int round);

int fegetenv(fenv_t *envp);
int feholdexcept(fenv_t *envp);
int fesetenv(const fenv_t *envp);
int feupdateenv(const fenv_t *envp);

#ifdef __cplusplus
}
#endif

#endif /* !_UNIX_FENV_H */
