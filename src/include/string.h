#ifndef UNIX_STRING_H
#define UNIX_STRING_H 1

#include "include:string.h"

/* Nonstandard routines */
#if !defined(_ANSI_SOURCE) && !defined(_STRICT_ANSI)

#ifdef NO_MACROS
#undef index
#undef rindex
#undef bcopy
#undef bcmp
#undef bzero
extern char *index(const char *, int);
extern char *rindex(const char *, int);
extern void bcopy(const void *, void *, size_t);
extern int bcmp(const void *, const void *, size_t);
extern void bzero(void *, size_t);
#endif

/* Case-insensitive string comparison */
extern int strcasecmp(const char *, const char *);
extern int strncasecmp(const char *, const char *, size_t);

/* Memory-safe string functions using AmigaOS native functions */
extern char *strncpy(char *, const char *, size_t);
extern char *strncat(char *, const char *, size_t);
extern int stricmp(const char *, const char *);
extern int strnicmp(const char *, const char *, size_t);

/* BSD extensions for safer string handling */
extern size_t strlcpy(char *, const char *, size_t);
extern size_t strlcat(char *, const char *, size_t);
extern char *strsep(char **, const char *);

/* POSIX extensions */
extern size_t strnlen(const char *, size_t);
extern int strcoll(const char *, const char *);

/* Additional POSIX functions */
extern char *strndup(const char *, size_t);
extern char *strtok_r(char *, const char *, char **);
extern int strerror_r(int, char *, size_t);

/* Final C99 functions */
extern void *memccpy(void *, const void *, int, size_t);
extern size_t strxfrm(char *, const char *, size_t);

#endif
#endif
