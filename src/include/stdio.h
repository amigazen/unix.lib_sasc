#ifndef _UNIX_STDIO_H
#define _UNIX_STDIO_H
#include "include:stdio.h"
extern int pclose(FILE *);
extern FILE *popen(const char *, const char *);
extern char *tempnam(const char *, const char *);

/* These snprintf functions use utility.library and RawDoFmt() by default only falling back to the __versions if needed */
extern int snprintf(char *buffer, size_t bufsize, const char *fmt, ...);
extern int vsnprintf(char *buffer, size_t bufsize, const char *fmt, va_list args);

/* These are the fallback versions that do their own format processing */
extern int __snprintf(char *buffer, size_t bufsize, const char *fmt, ...);
extern int __vsnprintf(char *buffer, size_t bufsize, const char *fmt, va_list args);
#endif
