#ifndef LVO_CLIB_H__
#define LVO_CLIB_H__

#include <clib/compiler-specific.h>
#include <proto/exec.h>
#include <exec/types.h>
#include "startup.h"

/* POSIX regex functions with registerized prototypes */
int __ASM__ regcomp(register __a6 struct RegexBase *rb, register __a0 void *preg, register __a1 const char *pattern, register __d0 int cflags);
int __ASM__ regexec(register __a6 struct RegexBase *rb, register __a0 const void *preg, register __a1 const char *string, 
                   register __d0 ULONG nmatch, register __d1 void *pmatch, register __d2 int eflags);
ULONG __ASM__ regerror(register __a6 struct RegexBase *rb, register __d0 int errcode, register __a0 const void *preg, 
                       register __a1 char *errbuf, register __d1 ULONG errbuf_size);
void __ASM__ regfree(register __a6 struct RegexBase *rb, register __a0 void *preg);

/* ARexx interface */
LONG __ASM__ rematch(register __a6 struct RegexBase *rb, register __a0 STRPTR regex, register __a1 STRPTR string, 
                    register __d0 LONG flags, register __a2 void *pmatch);

/* Internal functions */
const char *__ASM__ regex_error_string(register __a6 struct RegexBase *rb, register __d0 int error);

#endif /* LVO_CLIB_H__ */
