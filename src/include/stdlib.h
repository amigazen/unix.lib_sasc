#ifndef _UNIX_STDLIB_H
#define _UNIX_STDLIB_H
#define unsetenv __fake_unsetenv

#include "include/stdint.h"
#include "include:stdlib.h"
#undef unsetenv
#ifndef _ANSI_SOURCE
int setenv(const char *, const char *, int);
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

#endif
#endif
