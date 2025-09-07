/*
 * regex.library startup header
 */

#ifndef STARTUP_H
#define STARTUP_H

/****************************************************************************/

#ifndef PROTO_EXEC_H
#define __USE_SYSBASE
#include <proto/exec.h>
#endif /* PROTO_EXEC_H */

#ifndef PROTO_DOS_H
#include <proto/dos.h>
#endif /* PROTO_DOS_H */

/****************************************************************************/

struct RegexBase
{
    struct Library          reb_Library;
    struct Library *        reb_SysBase;
    struct Library *        reb_DOSBase;
    BPTR                    reb_LibrarySegment;
    struct SignalSemaphore  reb_LockSemaphore;
};

#define SysBase (rb->reb_SysBase)
#define DOSBase (rb->reb_DOSBase)

/****************************************************************************/

#endif /* STARTUP_H */
