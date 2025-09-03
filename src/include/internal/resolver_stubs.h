/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * internal/resolver_stubs.h - Internal resolver stubs for Amiga
 * 
 * This header provides internal resolver stubs for the psockets implementation.
 * These are implementation details and should not be used by external code.
 */

#ifndef _RESOLVER_STUBS_H
#define _RESOLVER_STUBS_H

#include <netdb.h>
#include <sys/time.h> /* For struct timeval */

/*
 * Resolver state structure
 */
struct __res_state {
    int options;
};

/*
 * Global resolver state
 */
extern struct __res_state _res;

/*
 * Global host error variable
 */
extern int h_errno;

/*
 * Resolver options
 */
#define RES_INIT        0x00000001
#define RES_USE_INET6   0x00002000

/*
 * Function prototypes
 */
int res_init(void);

/*
 * Additional function prototypes needed by getaddrinfo.c
 * Note: These are POSIX-compliant wrappers that will be implemented in resolver_stubs.c
 */
struct hostent *gethostbyname(const char *name);
struct hostent *gethostbyaddr(const void *addr, unsigned long len, int type);
struct servent *getservbyname(const char *name, const char *proto);
struct servent *getservbyport(int port, const char *proto);
int inet_pton(int af, const char *src, void *dst);
const char *inet_ntop(int af, const void *src, char *dst, size_t size);
void freeaddrinfo(struct addrinfo *ai);
int snprintf(char *str, unsigned long size, const char *format, ...);
int select(int nfds, void *readfds, void *writefds, void *exceptfds, struct timeval *timeout);

#endif /* _RESOLVER_STUBS_H */
