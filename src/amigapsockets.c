/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * amigapsockets.c - Amiga bsdsocket library wrappers for psockets
 * Provides the actual calls to the Amiga system functions
 * 
 */

#include <clib/bsdsocket_protos.h>
#include <proto/bsdsocket.h>

/*
 * Amiga bsdsocket library function wrappers
 * These call the actual Amiga system functions with the correct signatures
 * 
 * Note: These functions are suffixed with _amiga to distinguish them
 * from the POSIX-compliant versions that are be provided by psockets.lib
 */

struct hostent *gethostbyname_amiga(STRPTR name)
{
    /* Call the actual Amiga bsdsocket library function */
    /* This is the real Amiga system call from bsdsocket.library */
    return gethostbyname(name);
}

struct hostent *gethostbyaddr_amiga(APTR addr, LONG len, LONG type)
{
    /* Call the actual Amiga bsdsocket library function */
    return gethostbyaddr(addr, len, type);
}

struct servent *getservbyname_amiga(STRPTR name, STRPTR proto)
{
    /* Call the actual Amiga bsdsocket library function */
    return getservbyname(name, proto);
}

struct servent *getservbyport_amiga(LONG port, STRPTR proto)
{
    /* Call the actual Amiga bsdsocket library function */
    return getservbyport(port, proto);
}

LONG inet_pton_amiga(LONG af, STRPTR src, APTR dst)
{
    /* Call the actual Amiga bsdsocket library function */
    return inet_pton(af, src, dst);
}

STRPTR inet_ntop_amiga(LONG af, APTR src, STRPTR dst, LONG size)
{
    /* Call the actual Amiga bsdsocket library function */
    return inet_ntop(af, src, dst, size);
}

VOID freeaddrinfo_amiga(struct addrinfo *ai)
{
    /* Call the actual Amiga bsdsocket library function */
    freeaddrinfo(ai);
}
