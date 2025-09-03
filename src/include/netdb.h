/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * netdb.h - POSIX network database functions for Amiga
 * 
 * This header provides POSIX-compliant network database functions
 * that wrap the native Amiga bsdsocket.library functions.
 */

#ifndef _UNIX_NETDB_H
#define _UNIX_NETDB_H

/* Include the system netdb.h first */
#include "netinclude:netdb.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Override/add psockets functions when HAVE_PSOCKETS is defined */
#ifdef HAVE_PSOCKETS

/* POSIX-compliant address resolution functions */
extern int getaddrinfo(const char *hostname, const char *servname, 
                      const struct addrinfo *hints, struct addrinfo **res);
extern int getnameinfo(const struct sockaddr *sa, socklen_t salen,
                      char *host, size_t hostlen, char *serv, size_t servlen, int flags);
extern void freeaddrinfo(struct addrinfo *ai);
extern const char *gai_strerror(int ecode);

#endif /* HAVE_PSOCKETS */

#ifdef __cplusplus
}
#endif

#endif /* _UNIX_NETDB_H */
