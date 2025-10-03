/* ctype.h - ANSI C character classification and conversion functions */

#ifdef __SASC
/* Include SAS/C's built-in ctype.h */
#include "include:ctype.h"
#else
#ifndef CTYPE_H
#define CTYPE_H

#include <stddef.h>

/* Character classification functions - declared as extern to allow macro override */
extern int isalnum(int c);
extern int isalpha(int c);
extern int iscntrl(int c);
extern int isdigit(int c);
extern int isgraph(int c);
extern int islower(int c);
extern int isprint(int c);
extern int ispunct(int c);
extern int isspace(int c);
extern int isupper(int c);
extern int isxdigit(int c);

/* Macro definitions for character classification */
#define isdigit(c)  ((_ctype)[((c)&0xff)] & (_DIGIT))

/* Character conversion functions - declared as extern to allow macro override */
extern int tolower(int c);
extern int toupper(int c);

/* Non-ANSI extensions (for compatibility) */
extern char _ctype[256];

#define _UPPER       (1)
#define _LOWER       (1 << 1)
#define _HEXIT       (1 << 2)
#define _DIGIT       (1 << 3)
#define _SPACE       (1 << 4)
#define _CNTRL       (1 << 5)
#define _PUNCT       (1 << 6)
#define _OCTIT       (1 << 7)

/* Macro implementations for efficiency - safe versions with proper parentheses */
#define isupper(x)   ((_ctype)[((x)&0xff)] & (_UPPER))
#define islower(x)   ((_ctype)[((x)&0xff)] & (_LOWER))
#define isxdigit(x)  ((_ctype)[((x)&0xff)] & (_HEXIT))
#define isdigit(x)   ((_ctype)[((x)&0xff)] & (_DIGIT))
#define isspace(x)   ((_ctype)[((x)&0xff)] & (_SPACE))
#define iscntrl(x)   ((_ctype)[((x)&0xff)] & (_CNTRL))
#define ispunct(x)   ((_ctype)[((x)&0xff)] & (_PUNCT))

#define isalpha(x)   ((_ctype)[((x)&0xff)] & ((_UPPER) | (_LOWER)))
#define isalnum(x)   ((_ctype)[((x)&0xff)] & ((_UPPER) | (_LOWER) | (_DIGIT)))

#define isprint(x)   (!((_ctype)[((x)&0xff)] & (_CNTRL)))
#define isgraph(x)   ((!((_ctype)[((x)&0xff)] & (_CNTRL))) && (((x)) != ' '))

/* Non-ANSI extensions */
#define iswhite(x)   ((_ctype)[((x)&0xff)] & (_SPACE))
#define isascii(x)   (((x) >= 0) && ((x) < 128))
#define iscsym(x)    (((_ctype)[((x)&0xff)] & ((_UPPER) | (_LOWER) | (_DIGIT))) || (((x)) == '_') || (((x)) == '$'))
#define isodigit(x)  ((_ctype)[((x)&0xff)] & (_OCTIT))

/* Safe conversion macros that handle EOF properly */
#define tolower(x)   (((x) == EOF) ? (EOF) : ((x) | 32))
#define toupper(x)   (((x) == EOF) ? (EOF) : ((x) & ~32))
#define tocntrl(x)   (((((x)+1)&~96)-1)&127)
#define toascii(x)   ((x) & 127)
#define toint(x)     ((int)(((_ctype)[((x)&0xff)]&(_DIGIT))?(((x)-'0')):((((x)|32)-'a'+10)))

#endif /* CTYPE_H */

#endif /* __SASC */