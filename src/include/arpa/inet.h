/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * arpa/inet.h - POSIX inet functions for Amiga
 * 
 * This header provides POSIX-compliant inet functions that wrap
 * the native Amiga bsdsocket.library functions.
 */

#ifndef _UNIX_ARPA_INET_H
#define _UNIX_ARPA_INET_H

/* Include the system arpa/inet.h first */
#include "netinclude:arpa/inet.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Override/add psockets functions when HAVE_PSOCKETS is defined */
#ifdef HAVE_PSOCKETS

/* POSIX-compliant inet functions */
extern int inet_pton(int af, const char *src, void *dst);
extern const char *inet_ntop(int af, const void *src, char *dst, size_t size);
extern int inet_aton(const char *cp, struct in_addr *ap);

#endif /* HAVE_PSOCKETS */

#ifdef __cplusplus
}
#endif

#endif /* _UNIX_ARPA_INET_H */
