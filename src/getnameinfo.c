/*
 * Copyright (C) 1995, 1996, 1997, and 1998 WIDE Project.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Simplified getnameinfo implementation for Amiga
 * Based on NetBSD implementation, IPv4-only, ANSI C compliant
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>

#ifdef AMITCP
#include "include/internal/resolver_stubs.h"
#endif

/* Define EAI_* constants if not already defined */
#ifndef EAI_ADDRFAMILY
#define EAI_ADDRFAMILY	1
#endif
#ifndef EAI_AGAIN
#define EAI_AGAIN	2
#endif
#ifndef EAI_BADFLAGS
#define EAI_BADFLAGS	3
#endif
#ifndef EAI_FAIL
#define EAI_FAIL	4
#endif
#ifndef EAI_FAMILY
#define EAI_FAMILY	5
#endif
#ifndef EAI_MEMORY
#define EAI_MEMORY	6
#endif
#ifndef EAI_NODATA
#define EAI_NODATA	7
#endif
#ifndef EAI_NONAME
#define EAI_NONAME	8
#endif
#ifndef EAI_SERVICE
#define EAI_SERVICE	9
#endif
#ifndef EAI_SOCKTYPE
#define EAI_SOCKTYPE	10
#endif
#ifndef EAI_SYSTEM
#define EAI_SYSTEM	11
#endif
#ifndef EAI_BADHINTS
#define EAI_BADHINTS	12
#endif
#ifndef EAI_PROTOCOL
#define EAI_PROTOCOL	13
#endif
#ifndef EAI_OVERFLOW
#define EAI_OVERFLOW	14
#endif
#ifndef EAI_MAX
#define EAI_MAX		15
#endif

/* Define NI_* constants if not already defined */
#ifndef NI_NUMERICHOST
#define NI_NUMERICHOST	0x00000001
#endif
#ifndef NI_NUMERICSERV
#define NI_NUMERICSERV	0x00000002
#endif
#ifndef NI_NOFQDN
#define NI_NOFQDN	0x00000004
#endif
#ifndef NI_NAMEREQD
#define NI_NAMEREQD	0x00000008
#endif
#ifndef NI_DGRAM
#define NI_DGRAM	0x00000010
#endif

/* Define AF_* constants if not already defined */
#ifndef AF_INET
#define AF_INET		2
#endif
#ifndef AF_INET6
#define AF_INET6	10
#endif

/* Define h_errno constants if not already defined */
#ifndef HOST_NOT_FOUND
#define HOST_NOT_FOUND	1
#endif
#ifndef TRY_AGAIN
#define TRY_AGAIN	2
#endif
#ifndef NO_RECOVERY
#define NO_RECOVERY	3
#endif
#ifndef NO_DATA
#define NO_DATA		4
#endif

/* Helper function to handle IPv4 addresses */
static int
getnameinfo_inet(const struct sockaddr *sa, socklen_t salen,
    char *host, socklen_t hostlen,
    char *serv, socklen_t servlen,
    int flags)
{
	struct servent *sp;
	struct hostent *hp;
	u_short port;
	const char *addr;
	char numserv[512];
	char numaddr[512];
	char *ptr;

	if (sa == NULL)
		return EAI_FAIL;

	if (sa->sa_family != AF_INET)
		return EAI_FAMILY;

	if (salen < sizeof(struct sockaddr_in))
		return EAI_FAMILY;

	/* Get port and address */
	port = ((struct sockaddr_in *)sa)->sin_port;
	addr = (const char *)&((struct sockaddr_in *)sa)->sin_addr;

	/* Handle service name/port */
	if (serv != NULL && servlen > 0) {
		if (flags & NI_NUMERICSERV) {
			snprintf(numserv, sizeof(numserv), "%u", ntohs(port));
			if (strlen(numserv) + 1 > servlen)
				return EAI_MEMORY;
			strncpy(serv, numserv, servlen - 1);
			serv[servlen - 1] = '\0';
		} else {
			sp = getservbyport(port, (flags & NI_DGRAM) ? "udp" : "tcp");
			if (sp != NULL && sp->s_name != NULL) {
				if (strlen(sp->s_name) + 1 > servlen)
					return EAI_MEMORY;
				strncpy(serv, sp->s_name, servlen - 1);
				serv[servlen - 1] = '\0';
			} else {
				snprintf(numserv, sizeof(numserv), "%u", ntohs(port));
				if (strlen(numserv) + 1 > servlen)
					return EAI_MEMORY;
				strncpy(serv, numserv, servlen - 1);
				serv[servlen - 1] = '\0';
			}
		}
	}

	/* Handle host name/address */
	if (host != NULL && hostlen > 0) {
		if (flags & NI_NUMERICHOST) {
			/* NUMERICHOST and NAMEREQD conflict with each other */
			if (flags & NI_NAMEREQD)
				return EAI_NONAME;

			if (inet_ntop(AF_INET, addr, numaddr, sizeof(numaddr)) == NULL)
				return EAI_SYSTEM;
			if (strlen(numaddr) + 1 > hostlen)
				return EAI_MEMORY;
			strncpy(host, numaddr, hostlen - 1);
			host[hostlen - 1] = '\0';
		} else {
			hp = gethostbyaddr(addr, sizeof(struct in_addr), AF_INET);
			if (hp != NULL && hp->h_name != NULL) {
				if (flags & NI_NOFQDN) {
					ptr = strchr(hp->h_name, '.');
					if (ptr != NULL)
						*ptr = '\0';
				}
				if (strlen(hp->h_name) + 1 > hostlen)
					return EAI_MEMORY;
				strncpy(host, hp->h_name, hostlen - 1);
				host[hostlen - 1] = '\0';
			} else {
				if (flags & NI_NAMEREQD)
					return EAI_NONAME;
				if (inet_ntop(AF_INET, addr, host, hostlen) == NULL)
					return EAI_SYSTEM;
			}
		}
	}

	return 0;
}

int
getnameinfo(const struct sockaddr *sa, socklen_t salen,
    char *host, socklen_t hostlen,
    char *serv, socklen_t servlen,
    int flags)
{
	if (sa == NULL)
		return EAI_FAIL;

	/* Check for invalid flags */
	if (flags & ~(NI_NUMERICHOST | NI_NUMERICSERV | NI_NOFQDN | NI_NAMEREQD | NI_DGRAM))
		return EAI_BADFLAGS;

	/* Check for conflicting flags */
	if ((flags & NI_NUMERICHOST) && (flags & NI_NAMEREQD))
		return EAI_BADFLAGS;

	switch (sa->sa_family) {
	case AF_INET:
		return getnameinfo_inet(sa, salen, host, hostlen, serv, servlen, flags);
	case AF_INET6:
		/* IPv6 not supported on Amiga - return appropriate error */
		return EAI_FAMILY;
	default:
		return EAI_FAMILY;
	}
}