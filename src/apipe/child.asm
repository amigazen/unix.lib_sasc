********************************************************************************
*
*  child.asm --- child process entry point for ExecCommand()
*
*  Copyright (C) 1991 by Per Bojsen.  All Rights Reserved.
*
*  Permission is granted to any individual or institution to use, copy,
*  modify, and distribute this software, provided that this complete
*  copyright and permission notice is maintained, intact, in all copies
*  and supporting documentation.
*
*  This software is provided on an "as is" basis without express or
*  implied warranty.
*
*  Main entry point is `__ChildProcess', referred to as
*  `int _ChildProcess(void)' from C.
*
*  $Id: child.asm,v 1.2 91/10/12 14:47:06 bojsen Exp Locker: bojsen $
*
*  $Log:        child.asm,v $
*  Revision 1.2  91/10/12  14:47:06  bojsen
*  Changed name of ChildProcess() to _ChildProcess() to avoid possible
*  collisions with client code.
*
*  Revision 1.1  91/10/12  14:39:53  bojsen
*  Initial revision
*

        INCLUDE 'exec/types.i'
        INCLUDE 'exec/libraries.i'
        INCLUDE 'exec/ports.i'

        INCLUDE 'dos/dos.i'
        INCLUDE 'dos/dosextens.i'
        INCLUDE 'dos/dostags.i'

*
* ChildMsg structure
*
* C equivalent:
*
*   struct ChildMsg
*   {
*     struct Message  ExecMsg;
*     char           *CmdLine;
*     LONG            StackSize;
*     BPTR            PathList;
*     ULONG           Flags;
*     LONG            RC;
*   };
*

 STRUCTURE CHM,MN_SIZE
    APTR   CHM_CmdLine   ; command line to execute
    LONG   CHM_StackSize ; use this stacksize for the child
    BPTR   CHM_PathList  ; path to install for child
    ULONG  CHM_Flags     ; various flags
    LONG   CHM_RC        ; child's return code
    LABEL  CHM_Size

 BITDEF CHM,PIPE,0
 BITDEF CHM,PATH,1

ABSEXECBASE EQU 4
LIBVERSION  EQU 37

        SECTION butility.lib_text,CODE

*
* Entry point
*
* Register usage:
*
*       A6 Current library
*       A5 Process pointer; process port; Child message pointer
*       A4 CLI pointer
*       A0 scratch
*
*       D4 Old path list of process
*       D2

        xdef __ChildProcess

_LVOOpenLibrary         EQU     -552
_LVOCloseLibrary        EQU     -414
_LVOFindTask            EQU     -294
_LVOForbid              EQU     -132
_LVOWaitPort            EQU     -384
_LVOGetMsg              EQU     -372
_LVOReplyMsg            EQU     -378
_LVOSystemTagList       EQU     -606
_LVOInput               EQU     -54
_LVORead                EQU     -42

__ChildProcess:
        movem.l D1-D4/A1/A4-A6,-(A7)
        move.l ABSEXECBASE.w,A6

        sub.l A1,A1
        jsr _LVOFindTask(A6)
        move.l D0,A5

        ; Get CLI pointer

        move.l pr_CLI(A5),D3

        ; Get startup message

        lea.l pr_MsgPort(A5),A5
        move.l A5,A0
        jsr _LVOWaitPort(A6)

        move.l A5,A0
        jsr _LVOGetMsg(A6)
        move.l D0,A5

        ; Open dos.library

        lea DOSLibraryName(PC),A1
        move.l #LIBVERSION,D0
        jsr _LVOOpenLibrary(A6)
        tst.l D0
        bne.s PrepSystem

        moveq #-1,D0
        move.l D0,CHM_RC(A5)
        bra FinishUp

PrepSystem:
        ; Switch path if one was passed in
        move.l CHM_Flags(A5),D1
        btst #CHMB_PATH,D1
        bne.s CheckCLI

        move.l #0,D3    ; clear CLI pointer so the path is not patched

CheckCLI:
        lsl.l #2,D3     ; convert BPTR to CLI
        move.l D3,A4
        beq.s DoSystem  ; was there a CLI?

        move.l CHM_PathList(A5),D1
        move.l cli_CommandDir(A4),D4 ; install the path that was passed
        move.l D1,cli_CommandDir(A4) ; in in the startup message

DoSystem:
        ; Build tag list for SystemTagList() on stack

        pea TAG_DONE
        move.l CHM_StackSize(A5),-(A7)
        pea NP_StackSize

        ; Now call SystemTagList() to run child

        move.l A7,D2
        move.l CHM_CmdLine(A5),D1
        move.l D0,A6
        jsr _LVOSystemTagList(A6)
        lea $C(A7),A7
        move.l D0,CHM_RC(A5)

        move.l A4,D1    ; did we have a CLI?
        beq.s FlushInput

        move.l D4,cli_CommandDir(A4) ; OK, reinstall the old path

FlushInput:
        ; Flush stdin if it's a pipe (look for PIPE flag)

        move.l CHM_Flags(A5),D0
        btst #CHMB_PIPE,D0 ; D0 is CHM_Flags(A5) at this point
        beq.s CloseDOS

        jsr _LVOInput(A6)
        move.l D0,D4
        beq.s CloseDOS

BUFFERSZ        EQU     512
        lea -BUFFERSZ(A7),A7
        move.l A7,D2
        move.l #BUFFERSZ,D3

FlushIn:
        move.l D4,D1
        jsr _LVORead(A6)
        neg.l D0
        bmi.s FlushIn

        lea BUFFERSZ(A7),A7

CloseDOS:
        ; Close dos.library

        move.l A6,A1
        move.l ABSEXECBASE.w,A6
        jsr _LVOCloseLibrary(A6)

FinishUp:
        jsr _LVOForbid(A6) ; Forbid() so the parent doesn't unload us too early

        move.l A5,A1
        jsr _LVOReplyMsg(A6)

        move.l #0,D0
        movem.l (A7)+,D1-D4/A1/A4-A6
        rts

DOSLibraryName
        dc.b 'dos.library',0

        end
