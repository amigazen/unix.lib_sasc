#ifndef _UNIX_LIMITS_H
#define _UNIX_LIMITS_H

#ifdef __SASC
/* Include SAS/C's built-in limits.h */
#include "sc:include/limits.h"
#else
#error Wrong compiler (SAS/C required)
#endif

#endif /* !_UNIX_LIMITS_H */
