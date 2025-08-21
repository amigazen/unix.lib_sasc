/* Avoid problems with conflicting declarations for mkdir */
#define mkdir __fake_mkdir
#define getenv __fake_getenv
#include "include:stdlib.h"
#undef mkdir
#undef getenv