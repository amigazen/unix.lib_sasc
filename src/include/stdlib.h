#ifndef _STDLIB_H
#define _STDLIB_H

/* Avoid problems with conflicting declarations for mkdir */
#define mkdir __fake_mkdir
#include "include:stdlib.h"
#undef mkdir

#endif /* _STDLIB_H */
