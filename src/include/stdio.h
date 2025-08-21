#ifndef _STDIO_H
#define _STDIO_H

/* Avoid problems with conflicting declarations for mkdir */
#define mkdir __fake_mkdir
#include "include:stdio.h"
#undef mkdir

#endif /* _STDIO_H */
