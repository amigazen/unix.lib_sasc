#ifndef _UNIX_CTYPE_H
#define _UNIX_CTYPE_H

#ifdef __SASC
/* Include SAS/C's built-in ctype.h */
#include "include:ctype.h"
#else
#error Wrong compiler (SAS/C required)
#endif

#endif /* !_UNIX_CTYPE_H */
