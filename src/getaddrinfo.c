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
 * Simplified getaddrinfo implementation for Amiga
 * Based on NetBSD implementation, IPv4-only, ANSI C compliant
 */

 #include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <arpa/inet.h>

#ifdef AMITCP
#include "include/internal/resolver_stubs.h"
/* Additional function prototypes for Amiga */
extern struct hostent *gethostbyname(const char *name);
extern struct servent *getservbyname(const char *name, const char *proto);
extern int inet_pton(int af, const char *src, void *dst);
extern int h_errno;
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

/* Define AI_* constants if not already defined */
#ifndef AI_PASSIVE
#define AI_PASSIVE	0x00000001
#endif
#ifndef AI_CANONNAME
#define AI_CANONNAME	0x00000002
#endif
#ifndef AI_NUMERICHOST
#define AI_NUMERICHOST	0x00000004
#endif
#ifndef AI_NUMERICSERV
#define AI_NUMERICSERV	0x00000008
#endif
#ifndef AI_MASK
#define AI_MASK		(AI_PASSIVE | AI_CANONNAME | AI_NUMERICHOST | AI_NUMERICSERV)
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

/* Define PF_* constants if not already defined */
#ifndef PF_UNSPEC
#define PF_UNSPEC	0
#endif
#ifndef PF_INET
#define PF_INET		2
#endif

/* Define AF_* constants if not already defined */
#ifndef AF_INET
#define AF_INET		2
#endif

/* Define SOCK_* constants if not already defined */
#ifndef SOCK_STREAM
#define SOCK_STREAM	1
#endif
#ifndef SOCK_DGRAM
#define SOCK_DGRAM	2
#endif
#ifndef SOCK_RAW
#define SOCK_RAW	3
#endif

 /* Error message strings */
 static const char * const ai_errlist[] = {
	 "Success",
	 "Address family for hostname not supported",	/* EAI_ADDRFAMILY */
	 "Temporary failure in name resolution",		/* EAI_AGAIN	  */
	 "Invalid value for ai_flags",			/* EAI_BADFLAGS	  */
	 "Non-recoverable failure in name resolution",	/* EAI_FAIL	  */
	 "ai_family not supported",			/* EAI_FAMILY	  */
	 "Memory allocation failure",			/* EAI_MEMORY	  */
	 "No address associated with hostname",		/* EAI_NODATA	  */
	 "hostname or servname not provided or not known", /* EAI_NONAME	  */
	 "servname not supported for ai_socktype",	/* EAI_SERVICE	  */
	 "ai_socktype not supported",			/* EAI_SOCKTYPE	  */
	 "System error returned in errno",		/* EAI_SYSTEM	  */
	 "Invalid value for hints",			/* EAI_BADHINTS	  */
	 "Resolved protocol is unknown",			/* EAI_PROTOCOL	  */
	 "Argument buffer overflow",			/* EAI_OVERFLOW	  */
	 "Unknown error",				/* EAI_MAX	  */
 };
 
 const char *
 gai_strerror(int ecode)
 {
	 if (ecode < 0 || ecode > EAI_MAX)
		 ecode = EAI_MAX;
	 return ai_errlist[ecode];
 }
 
 void
 freeaddrinfo(struct addrinfo *ai)
 {
	 struct addrinfo *next;
 
	 if (ai == NULL)
		 return;
 
	 do {
		 next = ai->ai_next;
		 if (ai->ai_canonname)
			 free(ai->ai_canonname);
		 if (ai->ai_addr)
			 free(ai->ai_addr);
		 free(ai);
		 ai = next;
	 } while (ai);
 }
 
 /* Helper function to convert string to number */
 static int
 str2number(const char *p)
 {
	 char *ep;
	 unsigned long v;
 
	 if (p == NULL || *p == '\0')
		 return -1;
	 
	 ep = NULL;
	 errno = 0;
	 v = strtoul(p, &ep, 10);
	 if (errno == 0 && ep && *ep == '\0' && v <= 65535)
		 return (int)v;
	 else
		 return -1;
 }
 
 /* Helper function to get port number */
 static int
 get_port(struct addrinfo *ai, const char *servname, int matchonly)
 {
	 struct servent *sp;
	 int port;
	 int allownumeric;
 
	 if (servname == NULL)
		 return 0;
 
		switch (ai->ai_family) {
			case AF_INET:
				break;
	 default:
		 return 0;
	 }
 
	 switch (ai->ai_socktype) {
	 case SOCK_RAW:
		 return EAI_SERVICE;
	 case SOCK_DGRAM:
	 case SOCK_STREAM:
		 allownumeric = 1;
		 break;
	 case 0: /* ANY */
		 allownumeric = 1;
				break;
	 default:
		 return EAI_SOCKTYPE;
	 }
 
	 port = str2number(servname);
	 if (port >= 0) {
		 if (!allownumeric)
			 return EAI_SERVICE;
		 if (port < 0 || port > 65535)
			 return EAI_SERVICE;
		 port = htons(port);
	 } else {
		 if (ai->ai_flags & AI_NUMERICSERV)
			 return EAI_NONAME;
 
		 sp = getservbyname(servname, 
			 (ai->ai_socktype == SOCK_DGRAM) ? "udp" : "tcp");
		 if (sp == NULL)
			 return EAI_SERVICE;
		 port = sp->s_port;
	 }
 
	 if (!matchonly) {
		 ((struct sockaddr_in *)ai->ai_addr)->sin_port = port;
	 }
	 return 0;
 }
 
 /* Helper function to create addrinfo structure */
 static struct addrinfo *
 get_ai(const struct addrinfo *pai, const char *addr)
 {
	 struct addrinfo *ai;
	 struct sockaddr_in *sinptr;
 
	 ai = calloc(1, sizeof(struct addrinfo));
	 if (ai == NULL)
		 return NULL;
 
	 sinptr = calloc(1, sizeof(struct sockaddr_in));
	 if (sinptr == NULL) {
		 free(ai);
		 return NULL;
	 }
 
	 /* Copy hints */
	 *ai = *pai;
	 ai->ai_addr = (struct sockaddr *)sinptr;
	 ai->ai_addrlen = sizeof(struct sockaddr_in);
	 ai->ai_family = AF_INET;
	 ai->ai_next = NULL;
	 ai->ai_canonname = NULL;
 
	 /* Set up socket address */
	 sinptr->sin_family = AF_INET;
	 memcpy(&sinptr->sin_addr, addr, sizeof(struct in_addr));
 
	 return ai;
 }
 
 /* Handle numeric hostname */
static int
 explore_numeric(const struct addrinfo *pai, const char *hostname,
	 const char *servname, struct addrinfo **res)
 {
	 struct addrinfo *cur;
	 struct addrinfo sentinel;
	 int error;
	 char pton[4];
 
	 *res = NULL;
	 sentinel.ai_next = NULL;
	 cur = &sentinel;
 
	 /* Check if servname matches socktype/protocol */
	 if (servname != NULL) {
		 struct addrinfo test_ai = *pai;
		 test_ai.ai_family = AF_INET;
		 error = get_port(&test_ai, servname, 1);
		 if (error != 0)
			 return 0;
	 }
 
	/* Try to parse as IPv4 address */
	if (inet_pton(AF_INET, hostname, pton) == 1) {
		 if (pai->ai_family == AF_INET || pai->ai_family == PF_UNSPEC) {
			 cur->ai_next = get_ai(pai, pton);
			 if (cur->ai_next == NULL) {
				 error = EAI_MEMORY;
				 goto free;
			 }
			 
			 error = get_port(cur->ai_next, servname, 0);
			 if (error != 0)
				 goto free;
 
			 if ((pai->ai_flags & AI_CANONNAME)) {
				 cur->ai_next->ai_canonname = strdup(hostname);
				 if (cur->ai_next->ai_canonname == NULL) {
					 error = EAI_MEMORY;
					 goto free;
				 }
			 }
			 /* cur = cur->ai_next; */ /* Not needed for single result */
		 } else {
			 error = EAI_FAMILY;
			 goto free;
		 }
	 } else {
		 error = EAI_NONAME;
		 goto free;
	 }
 
	 *res = sentinel.ai_next;
	 return 0;
 
 free:
	 if (sentinel.ai_next)
		 freeaddrinfo(sentinel.ai_next);
	 return error;
 }
 
 /* Handle NULL hostname */
static int
 explore_null(const struct addrinfo *pai, const char *servname,
	 struct addrinfo **res)
 {
	 struct addrinfo *cur;
	 struct addrinfo sentinel;
	 int error;
	 static const char in_addrany[] = { 0, 0, 0, 0 };
	 static const char in_loopback[] = { 127, 0, 0, 1 };
 
	 *res = NULL;
	 sentinel.ai_next = NULL;
	 cur = &sentinel;
 
	 /* Check if servname matches socktype/protocol */
	 if (servname != NULL) {
		 struct addrinfo test_ai = *pai;
		 test_ai.ai_family = AF_INET;
		 error = get_port(&test_ai, servname, 1);
		 if (error != 0)
			 return 0;
	 }
 
	 if (pai->ai_family != AF_INET && pai->ai_family != PF_UNSPEC)
		 return 0;
 
	 if (pai->ai_flags & AI_PASSIVE) {
		 cur->ai_next = get_ai(pai, in_addrany);
	 } else {
		 cur->ai_next = get_ai(pai, in_loopback);
	 }
	 
	 if (cur->ai_next == NULL) {
		 error = EAI_MEMORY;
		 goto free;
	 }
 
	 	 error = get_port(cur->ai_next, servname, 0);
	 if (error != 0)
		 goto free;

	 /* cur = cur->ai_next; */ /* Not needed for single result */
 
	 *res = sentinel.ai_next;
	 return 0;
 
 free:
	 if (sentinel.ai_next)
		 freeaddrinfo(sentinel.ai_next);
	 return error;
 }
 
 /* Handle FQDN hostname */
static int
 explore_fqdn(const struct addrinfo *pai, const char *hostname,
	 const char *servname, struct addrinfo **res)
 {
	 struct addrinfo *result;
	 struct addrinfo *cur;
	 int error;
	 struct hostent *hp;
 
	 *res = NULL;
 
	 /* Check if servname matches socktype/protocol */
	 if (servname != NULL) {
		 struct addrinfo test_ai = *pai;
		 test_ai.ai_family = AF_INET;
		 error = get_port(&test_ai, servname, 1);
		 if (error != 0)
			 return 0;
	 }
 
	 /* Look up hostname */
	 hp = gethostbyname(hostname);
	 if (hp == NULL) {
		 switch (h_errno) {
		 case HOST_NOT_FOUND:
			 return EAI_NONAME;
		 case TRY_AGAIN:
			 return EAI_AGAIN;
		 case NO_RECOVERY:
			 return EAI_FAIL;
		 case NO_DATA:
			 return EAI_NODATA;
		 default:
			 return EAI_NONAME;
		 }
	 }
 
	 /* Check address family */
	 if (hp->h_addrtype != AF_INET) {
		 return EAI_FAMILY;
	 }
 
	 /* Create addrinfo structures for each address */
	 result = NULL;
	 cur = NULL;
	 
	 {
		 char **ap;
		 for (ap = hp->h_addr_list; *ap != NULL; ap++) {
			 struct addrinfo *ai;
			 
			 ai = get_ai(pai, *ap);
			 if (ai == NULL) {
				 error = EAI_MEMORY;
				 goto free;
			 }
 
			 error = get_port(ai, servname, 0);
			 if (error != 0) {
				 freeaddrinfo(ai);
				 goto free;
			 }
 
			 if (result == NULL) {
				 result = ai;
				 cur = ai;
			 } else {
				 cur->ai_next = ai;
				 cur = ai;
			 }
		 }
	 }
 
	 /* Set canonical name if requested */
	 if (result && (pai->ai_flags & AI_CANONNAME)) {
		 result->ai_canonname = strdup(hp->h_name);
		 if (result->ai_canonname == NULL) {
			 error = EAI_MEMORY;
			 goto free;
		 }
	 }
 
	 *res = result;
	 return 0;
 
 free:
	 if (result)
		 freeaddrinfo(result);
	 return error;
 }
 
 int
 getaddrinfo(const char *hostname, const char *servname,
	 const struct addrinfo *hints, struct addrinfo **res)
 {
	 struct addrinfo sentinel;
	 struct addrinfo *cur;
	 int error;
	 struct addrinfo ai;
	 struct addrinfo *pai;
 
	 if (res == NULL)
		 return EAI_SYSTEM;
 
	 /* Initialize sentinel */
	 memset(&sentinel, 0, sizeof(sentinel));
	 cur = &sentinel;
	 memset(&ai, 0, sizeof(ai));
	 pai = &ai;
	 pai->ai_flags = 0;
	 pai->ai_family = PF_UNSPEC;
	 pai->ai_socktype = 0;
	 pai->ai_protocol = 0;
	 pai->ai_addrlen = 0;
	 pai->ai_canonname = NULL;
	 pai->ai_addr = NULL;
	 pai->ai_next = NULL;
 
	 if (hostname == NULL && servname == NULL)
		 return EAI_NONAME;
 
	 if (hints) {
		 /* Error check for hints */
		 if (hints->ai_addrlen || hints->ai_canonname ||
			 hints->ai_addr || hints->ai_next)
			 return EAI_BADHINTS;
		 if (hints->ai_flags & ~AI_MASK)
			 return EAI_BADFLAGS;
		 switch (hints->ai_family) {
		 case PF_UNSPEC:
		 case PF_INET:
			 break;
		 default:
			 return EAI_FAMILY;
		 }
		 memcpy(pai, hints, sizeof(*pai));
	 }
 
	 /* Handle NULL hostname */
	 if (hostname == NULL) {
		 error = explore_null(pai, servname, &cur->ai_next);
		 if (error)
			 goto free;
		 while (cur->ai_next)
			 cur = cur->ai_next;
		 goto good;
	 }
 
	 /* Try numeric hostname first */
	 error = explore_numeric(pai, hostname, servname, &cur->ai_next);
	 if (error == 0 && cur->ai_next) {
		 while (cur->ai_next)
			 cur = cur->ai_next;
		 goto good;
	 }
 
	 /* If AI_NUMERICHOST is set, don't try DNS */
	 if (pai->ai_flags & AI_NUMERICHOST)
		 return EAI_NONAME;
 
	 /* Try FQDN lookup */
	 error = explore_fqdn(pai, hostname, servname, &cur->ai_next);
	 if (error)
		 goto free;
 
	 while (cur && cur->ai_next)
		 cur = cur->ai_next;
 
 good:
	 if (sentinel.ai_next) {
		 *res = sentinel.ai_next;
		 return 0;
	 } else {
		 error = EAI_FAIL;
	 }
 
 free:
	 if (sentinel.ai_next)
		 freeaddrinfo(sentinel.ai_next);
	 *res = NULL;
	 return error;
 }