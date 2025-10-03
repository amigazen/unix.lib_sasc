/* varargs.h - pre-ANSI C variable argument macros */

#ifndef _UNIX_VARARGS_H
#define _UNIX_VARARGS_H

#ifndef VARARGS_H
#define VARARGS_H

/* Macros enabling functions to portably handle variable numbers of
 * arguments.
 */

#ifndef _VA_LIST_DEFINED
#define _VA_LIST_DEFINED
typedef         char *va_list;
#elif !defined(va_list)
typedef         char *va_list;
#endif

#define         va_decl         int va_alist;
#define         va_start(X)     (X = (char *) va_alist)
#define         va_arg(X,T)     ((T *)(X += sizeof(T)))[-1]
#define         va_end(X)

#endif /* VARARGS_H */

#endif /* !_UNIX_VARARGS_H */