/* Avoid problems with conflicting declarations for mkdir */
#define mkdir __fake_mkdir
#define chmod __fake_chmod
#define getcwd __fake_getcwd
#define perror __fake_perror
#define rename __fake_rename
#include "include:stdio.h"
#undef mkdir
#undef chmod
#undef getcwd
#undef perror
#undef rename
