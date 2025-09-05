#ifndef _UNIX_FLOAT_H
#define _UNIX_FLOAT_H

#ifdef __SASC
/* Include SAS/C's built-in float.h */
#include "sc:include/float.h"
#else
#error Wrong compiler (SAS/C required)
#endif

#endif /* !_UNIX_FLOAT_H */
