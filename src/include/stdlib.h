#ifndef _UNIX_STDLIB_H
#define _UNIX_STDLIB_H
#define unsetenv __fake_unsetenv

#include "stdint.h"
#ifdef __SASC
#include "sc:include/stdlib.h"
#else
#error Wrong compiler (SAS/C required)
#endif
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
