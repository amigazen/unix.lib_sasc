#ifndef _SYS_DIR_H_
#define _SYS_DIR_H_

/*
 * WARNING: This header is for backward compatibility with old source code.
 * The modern POSIX standard uses <dirent.h>.
 * All new code should #include <dirent.h> directly.
 */
#include <dirent.h>

/*
 * For compatibility with very old code, define the old struct name 'direct'
 * as an alias for the modern 'dirent'.
 */
#define direct dirent

#endif /* !_SYS_DIR_H_ */