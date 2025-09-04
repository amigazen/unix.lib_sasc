/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 amigazen project
 * 
 * Amiga system includes for hybrid pthread implementation
 */

#ifndef _AMIGAPTHREAD_H
#define _AMIGAPTHREAD_H

/* Standard C includes */
#include "amiga.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <time.h>

/* Amiga system includes */
#include <exec/types.h>
#include <exec/lists.h>
#include <exec/semaphores.h>
#include <exec/tasks.h>
#include <exec/nodes.h>
#include <exec/memory.h>
#include <exec/ports.h>
#include <exec/io.h>
#include <exec/interrupts.h>
#include <exec/execbase.h>

#include <dos/dos.h>
#include <dos/dostags.h>
#include <dos/dosasl.h>
#include <dos/rdargs.h>

#include <devices/timer.h>

#include <utility/tagitem.h>

/* Prototypes */
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/utility.h>
#include <clib/alib_protos.h>

/* Timer device prototypes */
#include <clib/timer_protos.h>
#include <pragmas/timer_pragmas.h>

/* Constructor support */
#include <constructor.h>

/* Use Amiga TimeVal to avoid conflicts with POSIX timeval */

/* Forward declarations for functions that might not have prototypes */

/* Process flags */
#ifndef PRF_CODE
#define PRF_CODE 0x00000001
#endif
#ifndef PRF_NOCLI
#define PRF_NOCLI 0x00000002
#endif
#ifndef PRF_SAVEIO
#define PRF_SAVEIO 0x00000004
#endif
#ifndef NOCMD
#define NOCMD 0
#endif

/* ProcessControlBlock structure (from arp.library) */
struct ProcessControlBlock {
    LONG pcb_StackSize;
    LONG pcb_Pri;
    ULONG pcb_Flags;
    LONG pcb_Reserved1;
    LONG pcb_Reserved2;
    LONG pcb_Reserved3;
    LONG pcb_Reserved4;
    APTR pcb_Entry;
    LONG pcb_Reserved5;
    APTR pcb_WBProcess;
};

/* ASyncRun function (from arp.library) - we'll implement our own */
extern LONG ASyncRun(STRPTR name, STRPTR cmd, struct ProcessControlBlock *pcb);

/* CreateNewProcTags prototype - most NP_* constants are in dos/dostags.h */
/* But NP_Flags is missing, so we define it */
#ifndef NP_Flags
#define NP_Flags        (TAG_USER + 5)
#endif
extern struct Process *CreateNewProcTags(ULONG tag1, ...);


/* Timer device constants */
#ifndef TIMERNAME
#define TIMERNAME "timer.device"
#endif

#ifndef UNIT_MICROHZ
#define UNIT_MICROHZ 0
#endif

#ifndef TR_ADDREQUEST
#define TR_ADDREQUEST 1
#endif

/* Signal constants */
#ifndef SIGBREAKF_CTRL_C
#define SIGBREAKF_CTRL_C (1L << 12)
#endif

/* Node types */
#ifndef NT_TASK
#define NT_TASK 1
#endif

#ifndef NT_MESSAGE
#define NT_MESSAGE 2
#endif

#ifndef NT_MSGPORT
#define NT_MSGPORT 3
#endif

#ifndef NT_SIGNALSEM
#define NT_SIGNALSEM 4
#endif

#endif /* _AMIGAPTHREAD_H */
