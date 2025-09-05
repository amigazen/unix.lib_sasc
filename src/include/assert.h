#ifndef _UNIX_ASSERT_H
#define _UNIX_ASSERT_H

#ifdef __SASC
/* Include SAS/C's built-in assert.h */
#include "sc:include/assert.h"
#else
#error Wrong compiler (SAS/C required)
#endif

#endif /* !_UNIX_ASSERT_H */
