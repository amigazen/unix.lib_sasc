/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * resolver_stubs.c - Minimal resolver stubs for Amiga
 * Provides minimal implementations of BSD resolver functions
 */

#include "include/internal/resolver_stubs.h"
#include <proto/utility.h>
#include <stdarg.h> // For va_list in snprintf
#include <devices/timer.h> // For struct __timeval

/* Forward declaration for Amiga system function */
extern LONG WaitSelect(LONG nfds, APTR read_fds, APTR write_fds, APTR except_fds, struct __timeval *timeout, ULONG *signals);

/*
 * Global resolver state
 */
struct __res_state _res = { 0 };

/*
 * Global host error variable
 * Note: h_errno is already defined in sockets.c, so we just declare it here
 */
extern int h_errno;

/*
 * Initialize resolver state
 */
int res_init(void)
{
    _res.options = RES_INIT;
    return 0;
}

/*
 * POSIX-compliant wrapper functions that call the Amiga bsdsocket library
 * These provide the standard POSIX interface while using the Amiga system calls
 */

/* Forward declarations for the Amiga wrapper functions */
/* These call the actual Amiga bsdsocket.library functions */
extern struct hostent *gethostbyname_amiga(STRPTR name);
extern struct hostent *gethostbyaddr_amiga(APTR addr, LONG len, LONG type);
extern struct servent *getservbyname_amiga(STRPTR name, STRPTR proto);
extern struct servent *getservbyport_amiga(LONG port, STRPTR proto);
extern LONG inet_pton_amiga(LONG af, STRPTR src, APTR dst);
extern STRPTR inet_ntop_amiga(LONG af, APTR src, STRPTR dst, LONG size);
/* freeaddrinfo is implemented in getaddrinfo.c */

/* POSIX-compliant wrapper functions */
/* These are the functions that will be linked by user code */
struct hostent *gethostbyname(const char *name)
{
    /* Convert POSIX const char* to Amiga STRPTR and call Amiga wrapper */
    return gethostbyname_amiga((STRPTR)name);
}

struct hostent *gethostbyaddr(const void *addr, unsigned long len, int type)
{
    /* Convert POSIX types to Amiga types and call Amiga wrapper */
    return gethostbyaddr_amiga((APTR)addr, (LONG)len, (LONG)type);
}

struct servent *getservbyname(const char *name, const char *proto)
{
    /* Convert POSIX const char* to Amiga STRPTR and call Amiga wrapper */
    return getservbyname_amiga((STRPTR)name, (STRPTR)proto);
}

struct servent *getservbyport(int port, const char *proto)
{
    /* Convert POSIX int to Amiga LONG and call Amiga wrapper */
    return getservbyport_amiga((LONG)port, (STRPTR)proto);
}

/* inet_pton function */
int inet_pton(int af, const char *src, void *dst)
{
    /* Convert POSIX types to Amiga types and call Amiga wrapper */
    return inet_pton_amiga((LONG)af, (STRPTR)src, (APTR)dst);
}

/* inet_ntop function */
const char *inet_ntop(int af, const void *src, char *dst, size_t size)
{
    /* Convert POSIX types to Amiga types and call Amiga wrapper */
    return inet_ntop_amiga((LONG)af, (APTR)src, (STRPTR)dst, (LONG)size);
}

/* freeaddrinfo is now implemented in getaddrinfo.c */

int snprintf(char *str, unsigned long size, const char *format, ...)
{
    /* Use Amiga utility.library SNPrintf */
    va_list args;
    int result;
    
    va_start(args, format);
    result = VSNPrintf(str, size, format, args);
    va_end(args);
    
    return result;
}

int select(int nfds, void *readfds, void *writefds, void *exceptfds, struct timeval *timeout)
{
    /* Use Amiga bsdsocket library WaitSelect */
    ULONG signals = 0;
    LONG result;
    
    /* The system timeval and POSIX timeval are compatible */
    result = WaitSelect((LONG)nfds, readfds, writefds, exceptfds, 
                       (struct __timeval *)timeout, &signals);
    
    return (int)result;
}


