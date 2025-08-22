/*
 * Originally FIONBIO was simply defined to be 0, changed for
 * compatibility with AmiTCP. (EF 16-Nov-96)
 */

/* When building with AMITCP, use netinclude headers directly */
#ifdef AMITCP
#include "netinclude:sys/filio.h"
#else
/* Fallback definitions when not using AmiTCP */
#include "netinclude:sys/ioccom.h"
#define IOCPARM_MASK	0x1fff
#define IOC_IN		0x80000000
#define _IOC(inout,group,num,len) \
	(inout | ((len & IOCPARM_MASK) << 16) | ((group) << 8) | (num))
#define _IOW(g,n,t)	_IOC(IOC_IN, (g), (n), sizeof(t))
#endif

/* Our additional definitions */
#define FIONBIO _IOW('f', 126, long) /* ioctl to change non blocking mode
					data (an int *) is a pointer a boolean,
					TRUE for non-blocking io */
