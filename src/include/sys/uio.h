#ifndef _SYS_UIO_H
#define _SYS_UIO_H
/*
 *      The struct iovec is normally found
 *      in Berkeley systems in <sys/uio.h>.
 *      See the readv(2) and writev(2)
 *      manual pages for details.
 */

#ifdef AMITCP
/* When building with AMITCP, use netinclude headers directly */
#include "netinclude:sys/uio.h"
#else
/* Fallback to local definition */
#include <sys/types.h>

struct iovec {
    caddr_t iov_base;
    int iov_len;
};
#endif

#endif
