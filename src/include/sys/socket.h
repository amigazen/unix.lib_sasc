#ifndef _SYS_SOCKET_H
#define _SYS_SOCKET_H

#ifdef AMITCP

/* When building with AMITCP, use netinclude headers directly */
#include "netinclude:sys/socket.h"

#else

/* Support for socket pairs only */
#define AF_UNIX 1
#define SOCK_STREAM 1

#endif

/* Our additional functions */
extern int socketpair(int, int, int, int *);

#endif
