#ifndef _UNIX_WCTYPE_H
#define _UNIX_WCTYPE_H

/* Include SAS/C's built-in wctype.h */
#ifdef __SASC
#include "sc:include/wctype.h"
#else
#error Wrong compiler (SAS/C required)
#endif  

#endif /* !_UNIX_WCTYPE_H */
