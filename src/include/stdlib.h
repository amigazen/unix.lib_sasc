#ifndef _UNIX_STDLIB_H
#define _UNIX_STDLIB_H
#define unsetenv __fake_unsetenv

#include "stdint.h"
#ifdef __SASC
/* Include SAS/C's built-in stdlib.h */
#include "sc:include/stdlib.h"
#else

#ifndef STDLIB_H
#define STDLIB_H

#include <stddef.h>

/* ANSI C required types */
typedef struct {
    int quot;
    int rem;
} div_t;

typedef struct {
    long quot;
    long rem;
} ldiv_t;

/* C99 long long division type */
typedef struct {
    long long quot;
    long long rem;
} lldiv_t;

/* ANSI C required constants */
#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0
#define RAND_MAX 32767

/* ANSI C required functions */
void abort(void);
int abs(int j);
int atexit(void (*func)(void));
double atof(const char *nptr);
int atoi(const char *nptr);
long atol(const char *nptr);
void *bsearch(const void *key, const void *base, size_t nmemb, size_t size,
              int (*compar)(const void *, const void *));
void *calloc(size_t nmemb, size_t size);
div_t div(int numer, int denom);
void exit(int status);
void free(void *ptr);
char *getenv(const char *name);
long labs(long j);
ldiv_t ldiv(long numer, long denom);
void *malloc(size_t size);
int mblen(const char *s, size_t n);
size_t mbstowcs(wchar_t *pwcs, const char *s, size_t n);
int mbtowc(wchar_t *pwc, const char *s, size_t n);
void qsort(void *base, size_t nmemb, size_t size,
           int (*compar)(const void *, const void *));
int rand(void);
void *realloc(void *ptr, size_t size);
void srand(unsigned int seed);
double strtod(const char *nptr, char **endptr);
long strtol(const char *nptr, char **endptr, int base);
unsigned long strtoul(const char *nptr, char **endptr, int base);
int system(const char *string);
size_t wcstombs(char *s, const wchar_t *pwcs, size_t n);
int wctomb(char *s, wchar_t wchar);

/* C99 additional functions */
long long llabs(long long j);
lldiv_t lldiv(long long numer, long long denom);
long long strtoll(const char *nptr, char **endptr, int base);
unsigned long long strtoull(const char *nptr, char **endptr, int base);
float strtof(const char *nptr, char **endptr);
long double strtold(const char *nptr, char **endptr);

/* Non-ANSI extensions for compatibility */
void _abort(void);

char *mktemp(char *);

#endif /* STDLIB_H */

#endif /* __SASC */
#undef unsetenv
#ifndef _ANSI_SOURCE
int setenv(const char *, const char *, int);
int system(const char *);
#ifndef _POSIX_SOURCE
extern	char *optarg;			/* getopt(3) external variables */
extern	int optind;
extern	int opterr;
int	getopt (int, char * const *, const char *);
int unsetenv(const char *);
#endif

/* CRC32 calculation functions */
extern uint32_t crc32(const void *, size_t);
extern uint32_t crc32_string(const char *);

/* Thread-safe random number generation */
extern int rand_r(unsigned int *);

/* Standard C library sorting function */
extern void qsort(void *, size_t, size_t, int (*)(const void *, const void *));

/* String conversion functions */
extern double atof(const char *);
extern double strtod(const char *, char **);
extern unsigned long strtoul(const char *, char **, int);


/* Helper functions */
extern int toint(char);

#endif
#endif
