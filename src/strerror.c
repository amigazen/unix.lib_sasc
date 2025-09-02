/*
 * strerror.c - get error message string (POSIX compliant)
 *
 * This function returns a pointer to a string that describes the error code
 * passed in the argument errnum. This implementation provides comprehensive
 * error messages and is self-contained.
 *
 * POSIX.1-2001, POSIX.1-2008
 */

#include <errno.h>
#include <stdio.h>

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) ((sizeof (x) / sizeof ((x)[0])))
#endif

/* Comprehensive error message array */
static const char * errmsg[] = {
    "Unknown",                    /* 0 */
    "Not owner",                  /* 1 - EPERM */
    "No such file or directory",  /* 2 - ENOENT */
    "No such process",            /* 3 - ESRCH */
    "Interrupted system call",    /* 4 - EINTR */
    "I/O error",                  /* 5 - EIO */
    "No such device or address",  /* 6 - ENXIO */
    "Arg list too long",          /* 7 - E2BIG */
    "Exec format error",          /* 8 - ENOEXEC */
    "Bad file number",            /* 9 - EBADF */
    "No children",                /* 10 - ECHILD */
    "No more processes",          /* 11 - EAGAIN */
    "Not enough core",            /* 12 - ENOMEM */
    "Permission denied",          /* 13 - EACCES */
    "Bad address",                /* 14 - EFAULT */
    "Block device required",      /* 15 - ENOTBLK */
    "Mount device busy",          /* 16 - EBUSY */
    "File exists",                /* 17 - EEXIST */
    "Cross-device link",          /* 18 - EXDEV */
    "No such device",             /* 19 - ENODEV */
    "Not a directory",            /* 20 - ENOTDIR */
    "Is a directory",             /* 21 - EISDIR */
    "Invalid argument",           /* 22 - EINVAL */
    "File table overflow",        /* 23 - ENFILE */
    "Too many open files",        /* 24 - EMFILE */
    "Not a typewriter",           /* 25 - ENOTTY */
    "Text file busy",             /* 26 - ETXTBSY */
    "File too large",             /* 27 - EFBIG */
    "No space left on device",    /* 28 - ENOSPC */
    "Illegal seek",               /* 29 - ESPIPE */
    "Read-only file system"       /* 30 - EROFS */
};

/*
 * strerror() - get error message string
 *
 * The strerror() function returns a pointer to a string that describes the
 * error code passed in the argument errnum.
 *
 * Parameters:
 *   errnum: error number
 *
 * Returns: pointer to error message string
 */
char *strerror(int errnum)
{
    static char buf[16];

    /* Check if error number is within our array bounds */
    if ((errnum >= 0) && (errnum < ARRAY_SIZE(errmsg))) {
        return (char *)errmsg[errnum];
    }

    /* For unknown error numbers, return a numeric string */
    sprintf(buf, "%d", errnum);
    return buf;
}
