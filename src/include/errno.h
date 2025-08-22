#ifndef  _ERRNO_H
#define  _ERRNO_H  1

/**
*
* The following symbols are the error codes returned by the UNIX system
* functions.  Typically, a UNIX function returns -1 when an error occurs,
* and the global integer named errno contains one of these values.
*
*/

/* Additional error codes not in the system errno.h */
#define EOSERR		-1	/* Operating system error */

/* Include the system errno.h for all the standard error codes */
#include "netinclude:sys/errno.h"

/* Additional declarations specific to our implementation */
extern int __near _OSERR;
extern int sys_nerr;
extern char *sys_errlist[];

#endif
