/*
 * regex.library base structure
 */

#ifndef REGEXBASE_H
#define REGEXBASE_H

#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/semaphores.h>

struct RegexBase {
    struct Library          reb_Library;
    struct Library *        reb_SysBase;
    struct Library *        reb_DOSBase;
    BPTR                    reb_LibrarySegment;
    struct SignalSemaphore  reb_LockSemaphore;
};

#define SysBase (rb->reb_SysBase)
#define DOSBase (rb->reb_DOSBase)

#endif /* REGEXBASE_H */
