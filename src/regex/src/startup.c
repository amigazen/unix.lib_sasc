/*
 * regex.library startup code
 * Amiga shared library boilerplate for dynamic loading
 */

#include <exec/resident.h>
#include <exec/libraries.h>
#include <exec/semaphores.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include "startup.h"
#include "regex.library_rev.h"
#include "lvo_clib.h"

/****************************************************************************/

LONG
ReturnError(void)
{
    return -1;
}

/****************************************************************************/

static struct RegexBase * __ASM__
LibInit(
    __REG__(d0, struct RegexBase *    rb),
    __REG__(a0, BPTR                 librarySegment),
    __REG__(a6, struct Library *     sBase))
{
    struct RegexBase * return_value = NULL;

    if (sBase->lib_Version >= 39)
    {
        SysBase = sBase;

        rb->reb_LibrarySegment = librarySegment;

        rb->reb_Library.lib_Revision = REVISION;

        InitSemaphore(&rb->reb_LockSemaphore);
        
        return_value = rb;
    }

    if (return_value == NULL)
    {
        FreeMem((BYTE *)rb - rb->reb_Library.lib_NegSize,
            rb->reb_Library.lib_NegSize + rb->reb_Library.lib_PosSize);
    }

    return return_value;
}

/****************************************************************************/

static struct RegexBase * __ASM__
LibOpen(__REG__(a6, struct RegexBase *rb))
{
    struct RegexBase * return_value = rb;
    BOOL is_first_opener;

    is_first_opener = (BOOL)(rb->reb_Library.lib_OpenCnt == 0);

    rb->reb_Library.lib_OpenCnt++;
    rb->reb_Library.lib_Flags &= ~LIBF_DELEXP;

    ObtainSemaphore(&rb->reb_LockSemaphore);

    /* Perform initial library initialization here */
    if (is_first_opener)
    {
        /* Open required libraries */
        DOSBase = OpenLibrary("dos.library", 37);
        if (!DOSBase)
        {
            return_value = NULL;
        }
    }

    if (return_value == NULL)
    {
        /* Perform any cleanup work here if opening failed */
        rb->reb_Library.lib_OpenCnt--;
    }

    ReleaseSemaphore(&rb->reb_LockSemaphore);

    return return_value;
}

/****************************************************************************/

static BPTR __ASM__
LibExpunge(__REG__(a6, struct RegexBase *rb))
{
    BPTR return_value = (BPTR)NULL;

    if (rb->reb_Library.lib_OpenCnt == 0)
    {
        return_value = rb->reb_LibrarySegment;

        Remove((struct Node *)rb);

        FreeMem((BYTE *)rb - rb->reb_Library.lib_NegSize,
            rb->reb_Library.lib_NegSize + rb->reb_Library.lib_PosSize);
    }
    else
    {
        rb->reb_Library.lib_Flags |= LIBF_DELEXP;
    }

    return return_value;
}

/****************************************************************************/

static BPTR __ASM__
LibClose(__REG__(a6, struct RegexBase *rb))
{
    BPTR return_value = NULL;

    ObtainSemaphore(&rb->reb_LockSemaphore);

    /* Perform final library cleanup */
    if (rb->reb_Library.lib_OpenCnt == 1)
    {
        CloseLibrary(DOSBase);
        DOSBase = NULL;
    }

    rb->reb_Library.lib_OpenCnt--;

    ReleaseSemaphore(&rb->reb_LockSemaphore);

    if (rb->reb_Library.lib_OpenCnt == 0 && (rb->reb_Library.lib_Flags & LIBF_DELEXP))
        return_value = LibExpunge(rb);

    return return_value;
}

/****************************************************************************/

static LONG
LibReserved(void)
{
    return 0;
}

/****************************************************************************/

static const APTR LibVectors[] =
{
    (APTR)LibOpen,
    (APTR)LibClose,
    (APTR)LibExpunge,
    (APTR)LibReserved,

    (APTR)regcomp,
    (APTR)regexec,
    (APTR)regerror,
    (APTR)regfree,
    (APTR)rematch,

    (APTR) -1
};

/****************************************************************************/

struct LibraryInitTable
{
    ULONG   lit_BaseSize;
    APTR *  lit_VectorTable;
    APTR    lit_InitTable;
    APTR    lit_InitRoutine;
};

static struct LibraryInitTable LibInitTable =
{
    sizeof(struct RegexBase),
    (APTR *)LibVectors,
    NULL,
    (APTR)LibInit
};

/****************************************************************************/

const struct Resident RomTag =
{
    RTC_MATCHWORD,
    (struct Resident *)&RomTag,
    (struct Resident *)&RomTag+1,    /* Right behind the RomTag */
    RTF_AUTOINIT,
    VERSION,
    NT_LIBRARY,
    0,
    "regex.library",
    VSTRING,
    &LibInitTable
};
