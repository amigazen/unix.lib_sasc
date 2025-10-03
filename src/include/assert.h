/* assert.h - ANSI C assertion facility */

#ifdef __SASC
/* Include SAS/C's built-in assert.h */
#include "sc:include/assert.h"
#else
#ifndef ASSERT_H
#define ASSERT_H

#include <stdio.h>
#include <stdlib.h>

#undef assert

#ifdef NDEBUG
#define assert(ignore) ((void)0)
#else
extern void _Assert(const char *);
#define _STR(x) _VAL(x)
#define _VAL(x) #x
#define assert(expr) \
    ((expr) ? (void)0 : _Assert(__FILE__ ":" _STR(__LINE__) " " #expr))
#endif

#endif /* ASSERT_H */

#endif /* __SASC */

