#ifndef  _ERRNO_H
#define  _ERRNO_H  1

/**
*
* The following symbols are the error codes returned by the UNIX system
* functions.  Typically, a UNIX function returns -1 when an error occurs,
* and the global integer named errno contains one of these values.
*
*/

/* Include the system errno.h for all the standard error codes */
#include "netinclude:sys/errno.h"

/* ANSI C required error numbers */
#ifndef EDOM
#define EDOM		1	/* Domain error */
#endif
#ifndef ERANGE
#define ERANGE		2	/* Range error */
#endif

/* Additional error codes not in the system errno.h */
#define EOSERR		-1	/* Operating system error */
#define EOVERFLOW	75	/* Value too large for defined data type */

/* Ensure common error codes are defined */
#ifndef ENOENT
#define ENOENT		2	/* No such file or directory */
#endif
#ifndef EFAULT
#define EFAULT		14	/* Bad address */
#endif
#ifndef EINVAL
#define EINVAL		22	/* Invalid argument */
#endif
#ifndef EAGAIN
#define EAGAIN		35	/* Resource temporarily unavailable */
#endif

/* C99 error codes */
#ifndef EILSEQ
#define EILSEQ		84	/* Illegal byte sequence */
#endif
#ifndef ENOTSUP
#define ENOTSUP		95	/* Operation not supported */
#endif



/* Additional declarations specific to our implementation */
extern int __near _OSERR;
extern int sys_nerr;
extern char *sys_errlist[];

#endif
