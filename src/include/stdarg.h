/* stdarg.h - ANSI C variable argument macros */

#ifndef _UNIX_STDARG_H
#define _UNIX_STDARG_H

#ifdef __SASC
/* Include SAS/C's built-in stdarg.h */
#include "sc:include/stdarg.h"
#else

#ifndef STDARG_H
#define STDARG_H

#include <stddef.h>

typedef char *va_list;

#define va_start(ap, parmN) ((ap) = (char *)&(parmN) + sizeof(parmN))
#define va_arg(ap, type)    (*(type *)((ap) += sizeof(type), (ap) - sizeof(type)))
#define va_end(ap)          ((void)0)

#endif /* STDARG_H */

#endif /* __SASC */

#endif /* !_UNIX_STDARG_H */
