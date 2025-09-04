/*
**  Pgmpipe.c --- main module of APipe-Handler
**
**  Copyright (C) 1991-1994 by Per Bojsen.  All Rights Reserved.
**
**  Permission is granted to any individual or institution to use, copy,
**  modify, and distribute this software, provided that this complete
**  copyright and permission notice is maintained, intact, in all copies
**  and supporting documentation.
**
**  This software is provided on an "as is" basis without express or
**  implied warranty.
**
**  NOTE: The code is reentrant.  Be careful when modifying it.
**
**  $Id: pgmpipe.c,v 1.8 94/11/08 01:17:23 bojsen Exp Locker: bojsen $
**
**  $Log:	pgmpipe.c,v $
**  Revision 1.8  94/11/08  01:17:23  bojsen
**  Updated to SAS/C 6 (very little modification required).
**  
**  Revision 1.7  92/09/12  16:49:11  bojsen
**  Improved cloning of file handles so that reading/writing will start
**  from current position.
**
**  Revision 1.6  92/02/20  23:46:11  bojsen
**  Fixed bug in handling multiple outstanding read packets add EOF.
**
**  Revision 1.5  91/11/24  17:01:25  bojsen
**  Made extraction of command line from file name more robust; the code now
**  handles the case where the handler name is not prepended.
**
**  Revision 1.4  91/11/24  02:22:39  bojsen
**  First released version.  Changed references to ChildProcess() to
**  _ChildProcess().
**
**  Revision 1.3  91/10/01  02:35:30  bojsen
**  Added code to clone current dir of requester process.  Changed code
**  to obtain pointer to requester process.
**
**  Revision 1.2  91/09/24  23:59:44  bojsen
**  Fixed typo in #include line.
**
**  Revision 1.1  91/09/24  23:54:06  bojsen
**  Initial revision
**
*/

#include <exec/types.h>
#include <exec/memory.h>
#include <utility/tagitem.h>
#include <clib/macros.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/dostags.h>
#include <dos/var.h>
#ifdef __SASC
#include <proto/exec.h>
#include <proto/dos.h>
#include <clib/alib_protos.h>
#endif /* __SASC */
#include <string.h>
#include "APipe-Handler_rev.h"

/*
 *  Version tagging.
 */
STATIC char VersionTag[] = VERSTAG;

/*
 *  Debugging.
 */
#ifdef DEBUG
#define DPrintF(args) do { KPrintF args; } while (0)
#else /* !DEBUG */
#define DPrintF(args)
#endif /* !DEBUG */

#define AbsExecBase 4L
#define LIB_VERSION_SUPPORTED 37L
#define PIPEBUFFER_SIZE 4096

/* defines for read/write status */
#define PPIPE_NOTOPEN   0
#define PPIPE_OPEN      4
#define PPIPE_READ      1
#define PPIPE_READOPEN  (PGMPIPE_READ | PGMPIPE_OPEN)
#define PPIPE_WRITE     2
#define PPIPE_WRITEOPEN (PGMPIPE_WRITE | PGMPIPE_OPEN)
#define PPIPE_RWMASK    3

/* defines for fh_Arg1 field of filehandles */
#define PPIPE_READER  1
#define PPIPE_WRITER  2

/* defines for closing variable */
#define PPIPE_C_OPEN          0
#define PPIPE_C_RDRCLOSING    1
#define PPIPE_C_WRTCLOSING    2
#define PPIPE_C_CLOSINGMASK   3
#define PPIPE_C_RDRCLOSED     4
#define PPIPE_C_WRTCLOSED     8
#define PPIPE_C_CLOSEDMASK  0xC

/*
 *  Parameter packet to child process.
 */
struct ChildMsg
{
  struct Message  ExecMsg;
  char           *CmdLine;   /* the command line to execute */
  LONG            StackSize; /* use this stacksize for child's child */
  BPTR            PathList;  /* path list for child */
  ULONG           Flags;     /* flags for child */
  LONG            RC;        /* return code from child process */
};

/* Defines for ChildMsg.Flags */
#define CHMF_PIPE 1L /* stdin of child is a pipe (of PIPE: type) */
#define CHMF_PATH 2L /* path is passed in in PathList */

/*
 *  Command path element.
 */
struct PathEntry
{
  BPTR pe_NextPathEntry;
  BPTR pe_PathLock;
};

/*
 *  The library bases must be global by AmigaOS programming standards,
 *  and the pragma libcall requires them.
 */
struct ExecBase *SysBase;
struct DosLibrary *DOSBase;


/*
 *  Prototypes for external functions.
 */
extern int _ChildProcess(void);

/*
 *  Prototypes for local functions.
 */
static ULONG CopyFromFIFO(UBYTE *FIFO, ULONG FIFOSize, UBYTE **Start,
                          UBYTE *End, ULONG *FIFOFill, UBYTE *Dest,
                          ULONG NumBytes);
static ULONG CopyToFIFO(UBYTE *FIFO, ULONG FIFOSize, UBYTE **Start, UBYTE **End,
                        ULONG *FIFOFill, UBYTE *Src, ULONG NumBytes);
static STRPTR GetCommandLine(STRPTR FileSpec, BSTR HandlerName);
static LONG ExecCommand(char *CmdLine, struct MsgPort **ParentPort,
                        struct Process *Requester, struct FileHandle *ChildIO,
                        LONG IoDirection);
static BPTR CloneProcessIO(struct Process *Friend, LONG IoDirection);
static BPTR ClonePathList(struct CommandLineInterface *Peer);
static void FreePathList(BPTR);

void
PgmPipe(void)
{
  struct DosLibrary *l_DOSBase;

  SysBase = *((struct ExecBase **) AbsExecBase); /* `open' exec.library */

  if (l_DOSBase =
        (struct DosLibrary *) OpenLibrary("dos.library", LIB_VERSION_SUPPORTED))
  {
    struct Process *ourProc, *theirProc;
    struct DosList *ourDevNode;
    struct DosPacket *packet, *writePkt = NULL, *readPkt = NULL;
    struct DosPacket *writeCls = NULL, *readCls = NULL;
    struct MsgPort *childPort = NULL;
    struct MsgPort *oldConTask = NULL;
    struct ChildMsg *childMsg = NULL;
    struct FileHandle *fhIncoming, *fhOutgoing;
    UBYTE *pipeBuffer;
    UBYTE *pbStart, *pbEnd;
    ULONG bytesReady, bytesR = 0, bytesW = 0;
    UWORD closing = PPIPE_C_RDRCLOSED | PPIPE_C_WRTCLOSED;
    UWORD readwrite = PPIPE_NOTOPEN;
    ULONG waitSigMask;
    struct MinList readQ, writeQ;
#ifdef DEBUG
    ULONG packetNum = 0;
#endif /* DEBUG */

    NewList((struct List *) &readQ);
    NewList((struct List *) &writeQ);

    DOSBase = l_DOSBase; /* patch the global library base */

    ourProc = (struct Process *) FindTask(NULL);
    waitSigMask = 1L << ourProc->pr_MsgPort.mp_SigBit;
    packet = WaitPkt(); /* get parameter packet */

    if ((pipeBuffer = AllocMem(PIPEBUFFER_SIZE, 0)) == NULL)
      ReplyPkt(packet, DOSFALSE, ERROR_NO_FREE_STORE);
    else
    {
      pbStart = pbEnd = pipeBuffer;
      bytesReady = 0;

      /*
       *  Currently we don't use the parameters provided in the parameter
       *  packet.  Since we want a new process for each instance of reference
       *  to our handler we don't patch the DeviceList node, but use the
       *  node to get the handler name.
       */

      ourDevNode = (struct DosList *) BADDR(packet->dp_Arg3);

      ReplyPkt(packet, DOSTRUE, packet->dp_Arg2);

      do
      {
        struct Node *node;
        char *cmdLine;
        LONG ioerr;

        if (readPkt && bytesReady == 0 && writePkt == NULL &&
            closing & PPIPE_C_WRTCLOSED)
        {
          ReplyPkt(readPkt, bytesR, 0);
          while (readPkt = (struct DosPacket *)
                             (node = RemHead((struct List *) &readQ),
                              node ? node->ln_Name : NULL))
            ReplyPkt(readPkt, 0, 0);
        }

#ifdef DEBUG
        if (readPkt || writePkt)
          DPrintF(("Event loop: readPkt == 0x%08lx writePkt == 0x%08lx\n",
                   readPkt, writePkt));
#endif /* DEBUG */

        if (packet = WaitPkt())
        {
          DPrintF(("Packet %ld, port 0x%08lx: ", ++packetNum, packet->dp_Port));

          switch (packet->dp_Type)
          {
            struct TagItem adoTags[2];

            case ACTION_FINDINPUT:
              DPrintF(("ACTION_FINDINPUT 0x%08lx 0x%08lx %s\n",
                       packet->dp_Arg1 << 2, packet->dp_Arg2 << 2,
                       (char *) (packet->dp_Arg3 << 2) + 1));

              if (readwrite != PPIPE_NOTOPEN)
              {
                ReplyPkt(packet, DOSFALSE, ERROR_OBJECT_IN_USE);
                break;
              }
              else
                readwrite = PPIPE_READ;

            case ACTION_FINDOUTPUT:
#ifdef DEBUG
              if (packet->dp_Type == ACTION_FINDOUTPUT)
                DPrintF(("ACTION_FINDOUTPUT 0x%08lx 0x%08lx %s\n",
                         packet->dp_Arg1 << 2, packet->dp_Arg2 << 2,
                         (char *) (packet->dp_Arg3 << 2) + 1));
#endif /* DEBUG */

              if (readwrite & PPIPE_OPEN)
              {
                ReplyPkt(packet, DOSFALSE, ERROR_OBJECT_IN_USE);
                break;
              }
              else if (readwrite != PPIPE_READ)
                readwrite = PPIPE_WRITE;

              fhIncoming = (struct FileHandle *) BADDR(packet->dp_Arg1);

              /*
               *  Obtain command line to execute.
               */
              cmdLine = GetCommandLine((STRPTR) BADDR(packet->dp_Arg3) + 1,
                                       ourDevNode->dol_Name);

              /*
               *  Find out who sent the packet.
               */
              if ((packet->dp_Port->mp_Flags & PF_ACTION) == PA_SIGNAL)
                theirProc = (struct Process *) packet->dp_Port->mp_SigTask;
              else
              {
                ReplyPkt(packet, DOSFALSE, ERROR_BAD_STREAM_NAME);
                readwrite = PPIPE_NOTOPEN;
                break;
              }

              adoTags[0].ti_Tag = ADO_FH_Mode;
              adoTags[0].ti_Data =
                readwrite == PPIPE_READ ? MODE_NEWFILE : MODE_OLDFILE;
              adoTags[1].ti_Tag = TAG_DONE;

              if ((fhOutgoing = AllocDosObject(DOS_FILEHANDLE, adoTags))
                    == NULL)
              {
                ReplyPkt(packet, DOSFALSE, ERROR_NO_FREE_STORE);
                readwrite = PPIPE_NOTOPEN;
                break;
              }

              fhOutgoing->fh_Link = NULL;
              fhOutgoing->fh_Port = fhIncoming->fh_Port = (struct MsgPort *) 0;
              fhOutgoing->fh_Type = &ourProc->pr_MsgPort;

              if (readwrite == PPIPE_READ)
              {
                fhIncoming->fh_Arg1 = PPIPE_READER;
                fhOutgoing->fh_Arg1 = PPIPE_WRITER;
              }
              else
              {
                fhIncoming->fh_Arg1 = PPIPE_WRITER;
                fhOutgoing->fh_Arg1 = PPIPE_READER;
              }

              oldConTask = SetConsoleTask(theirProc->pr_ConsoleTask);

              DPrintF(("Getting ready to spawn child . . .  "));

              if (ioerr = ExecCommand(cmdLine, &childPort, theirProc,
                                      fhOutgoing, readwrite))
              {
                SetConsoleTask(oldConTask);
                ReplyPkt(packet, DOSFALSE, ioerr);
                FreeDosObject(DOS_FILEHANDLE, fhOutgoing);
                readwrite = PPIPE_NOTOPEN;
                break;
              }
              SetConsoleTask(oldConTask);

              DPrintF(("Child spawned.\n"));

              waitSigMask |= 1L << childPort->mp_SigBit;

              closing = PPIPE_C_OPEN;
              readwrite |= PPIPE_OPEN;

              ReplyPkt(packet, DOSTRUE, packet->dp_Res2);

              break;

            case ACTION_READ:
              DPrintF(("ACTION_READ %ld 0x%08lx %ld\n", packet->dp_Arg1,
                       packet->dp_Arg2, packet->dp_Arg3));

              if (packet->dp_Arg2 == NULL || packet->dp_Arg3 < 0)
                ReplyPkt(packet, DOSFALSE, ERROR_BAD_NUMBER);
              else if (closing & PPIPE_C_RDRCLOSED)
                ReplyPkt(packet, DOSFALSE, ERROR_INVALID_LOCK);
              else if (packet->dp_Arg1 == PPIPE_WRITER)
                ReplyPkt(packet, 0, 0);
              else if (readPkt == NULL)
              {
                readPkt = packet;
                bytesR = 0;
              }
              else
                AddTail((struct List *) &readQ,
                        (struct Node *) packet->dp_Link);
              break;

            case ACTION_WRITE:
              DPrintF(("ACTION_WRITE %ld 0x%08lx %ld\n", packet->dp_Arg1,
                       packet->dp_Arg2, packet->dp_Arg3));

              if (packet->dp_Arg2 == NULL || packet->dp_Arg3 < 0)
                ReplyPkt(packet, DOSFALSE, ERROR_BAD_NUMBER);
              else if (closing & PPIPE_C_WRTCLOSED)
                ReplyPkt(packet, DOSFALSE, ERROR_INVALID_LOCK);
              else if (packet->dp_Arg1 == PPIPE_READER)
                ReplyPkt(packet, 0, 0);
              else if (writePkt == NULL)
              {
                writePkt = packet;
                bytesW = 0;
              }
              else
                AddTail((struct List *) &writeQ,
                        (struct Node *) packet->dp_Link);
              break;

            case ACTION_END:
              DPrintF(("ACTION_END %ld\n", packet->dp_Arg1));

              if (packet->dp_Arg1 == PPIPE_READER &&
                  (closing & PPIPE_C_RDRCLOSED) == 0)
              {
                closing |= PPIPE_C_RDRCLOSING;
                readCls = packet;
              }
              else if (packet->dp_Arg1 == PPIPE_WRITER &&
                       (closing & PPIPE_C_WRTCLOSED) == 0)
              {
                closing |= PPIPE_C_WRTCLOSING;
                writeCls = packet;
              }
              else
                ReplyPkt(packet, DOSTRUE, 0);
              break;

            case ACTION_IS_FILESYSTEM:
              DPrintF(("ACTION_IS_FILESYSTEM\n"));

              ReplyPkt(packet, DOSFALSE, 0);
              break;

            default:
              DPrintF(("%ld\n", packet->dp_Type));

              ReplyPkt(packet, DOSFALSE, ERROR_ACTION_NOT_KNOWN);
          }

          DPrintF(("\n"));
        }

        do
        {
          if (readPkt)
          {
            if (bytesReady > 0)
              bytesR += CopyFromFIFO(pipeBuffer, PIPEBUFFER_SIZE, &pbStart,
                                     pbEnd, &bytesReady,
                                     (UBYTE *) readPkt->dp_Arg2 + bytesR,
                                     readPkt->dp_Arg3 - bytesR);
            if (bytesR < readPkt->dp_Arg3 && writePkt)
            {
              ULONG bytes = MIN(writePkt->dp_Arg3 - bytesW,
                                readPkt->dp_Arg3 - bytesR);

              CopyMem((UBYTE *) writePkt->dp_Arg2 + bytesW,
                      (UBYTE *) readPkt->dp_Arg2 + bytesR, bytes);
              bytesR += bytes;
              if ((bytesW += bytes) == writePkt->dp_Arg3)
              {
                ReplyPkt(writePkt, writePkt->dp_Arg3, 0);
                writePkt = NULL;
              }
            }
            if (bytesR == readPkt->dp_Arg3)
            {
              ReplyPkt(readPkt, readPkt->dp_Arg3, 0);
              readPkt = NULL;
            }
          }
          if (readPkt == NULL &&
              (readPkt = (struct DosPacket *)
                           (node = RemHead((struct List *) &readQ),
                            node ? node->ln_Name : NULL)))
          {
            bytesR = 0;
          }
          if (writePkt == NULL &&
              (writePkt = (struct DosPacket *)
                            (node = RemHead((struct List *) &writeQ),
                             node ? node->ln_Name : NULL)))
          {
            bytesW = 0;
          }
        }
        while (readPkt && (writePkt || bytesReady > 0));

        if (readPkt == NULL && closing & PPIPE_C_RDRCLOSING)
        {
          closing &= ~PPIPE_C_RDRCLOSING;
          closing |= PPIPE_C_RDRCLOSED;
        }

        while (writePkt &&
               (closing & PPIPE_C_RDRCLOSED ||
                PIPEBUFFER_SIZE - bytesReady >= writePkt->dp_Arg3 - bytesW))
        {
          if (closing & PPIPE_C_RDRCLOSED)
          {
            ReplyPkt(writePkt, writePkt->dp_Arg3, 0);
            writePkt = NULL;
          }
          else
          {
            ULONG bytes = CopyToFIFO(pipeBuffer, PIPEBUFFER_SIZE, &pbStart,
                                     &pbEnd, &bytesReady,
                                     (UBYTE *) writePkt->dp_Arg2 + bytesW,
                                     writePkt->dp_Arg3 - bytesW);

            if ((bytesW += bytes) == writePkt->dp_Arg3)
            {
              ReplyPkt(writePkt, writePkt->dp_Arg3, 0);
              writePkt = NULL;
            }
          }
          if (writePkt == NULL &&
              (writePkt = (struct DosPacket *)
                            (node = RemHead((struct List *) &writeQ),
                             node ? node->ln_Name : NULL)))
          {
            bytesW = 0;
          }
        }

        if (writePkt == NULL && closing & PPIPE_C_WRTCLOSING)
        {
          closing &= ~PPIPE_C_WRTCLOSING;
          closing |= PPIPE_C_WRTCLOSED;
        }
      }
      while ((closing & PPIPE_C_RDRCLOSED) == 0 ||
             (closing & PPIPE_C_WRTCLOSED) == 0);

      /* clean up from child process */
      if (childPort)
      {
        waitSigMask = 1L << childPort->mp_SigBit;

        while ((childMsg = (struct ChildMsg *) GetMsg(childPort)) == NULL)
          Wait(waitSigMask);

        DeleteMsgPort(childPort);
      }
      if (childMsg) /* means the child process existed */
      {
        if (readCls)
          ReplyPkt(readCls, DOSTRUE, 0);
        if (writeCls)
          ReplyPkt(writeCls, DOSTRUE, 0);
        FreePathList(childMsg->PathList);
        FreeVec(childMsg); /* no use for return code currently */
      }
      FreeMem(pipeBuffer, PIPEBUFFER_SIZE);
    }

    CloseLibrary((struct Library *) l_DOSBase);

    DPrintF(("Handler exiting.\n"));
  }
} /* PgmPipe */


STATIC ULONG
CopyFromFIFO(UBYTE *FIFO, ULONG FIFOSize, UBYTE **Start, UBYTE *End,
             ULONG *FIFOFill, UBYTE *Dest, ULONG NumBytes)
{
  ULONG bytesCopied = 0, bytes;

  DPrintF(("CopyFromFIFO: 0x%08lx %ld", Dest, NumBytes));

  if (*FIFOFill > 0 && NumBytes > 0)
  {
    bytes = *Start >= End ? FIFO + FIFOSize - *Start : *FIFOFill;

    CopyMem(*Start, Dest, bytesCopied = MIN(bytes, NumBytes));
    NumBytes -= bytesCopied;
    *Start += bytesCopied;
    *FIFOFill -= bytesCopied;
  }
  if (*FIFOFill > 0 && NumBytes > 0)
  {
    bytes = End - FIFO; /* invariant: *Start == FIFO + FIFOSize */

    CopyMem(FIFO, Dest + bytesCopied, bytes = MIN(bytes, NumBytes));
    *Start = FIFO + bytes;
    *FIFOFill -= bytes;
    bytesCopied += bytes;
  }

  DPrintF((" %ld\n\n", bytesCopied));

  return bytesCopied;
} /* CopyFromFIFO */


STATIC ULONG
CopyToFIFO(UBYTE *FIFO, ULONG FIFOSize, UBYTE **Start, UBYTE **End,
           ULONG *FIFOFill, UBYTE *Src, ULONG NumBytes)
{
  ULONG bytesCopied = 0, bytes;

  DPrintF(("CopyToFIFO: 0x%08lx %ld", Src, NumBytes));

  if (*FIFOFill == 0)
    *Start = *End = FIFO; /* normalize for efficiency */

  if (*FIFOFill < FIFOSize && NumBytes > 0)
  {
    bytes = *End <= *Start ? FIFOSize - *FIFOFill : FIFO + FIFOSize - *End;

    CopyMem(Src, *End, bytesCopied = MIN(bytes, NumBytes));
    NumBytes -= bytesCopied;
    *End += bytesCopied;
    *FIFOFill += bytesCopied;
  }
  if (*FIFOFill < FIFOSize && NumBytes > 0)
  {
    bytes = *Start - FIFO; /* invariant: *End == FIFO + FIFOSize */

    CopyMem(Src + bytesCopied, FIFO, bytes = MIN(bytes, NumBytes));
    *End = FIFO + bytes;
    *FIFOFill += bytes;
    bytesCopied += bytes;
  }

  DPrintF((" %ld\n\n", bytesCopied));

  return bytesCopied;
} /* CopyFromFIFO */


#define GetCICh(String, Char) \
  ((Char) = *(String), (Char) += (Char) >= 'A' && (Char) <= 'Z' ? 0x20 : 0)

STATIC STRPTR
GetCommandLine(STRPTR FileSpec, BSTR HandlerName)
{
  UBYTE *cp1 = FileSpec, *cp2 = BADDR(HandlerName);
  LONG hNmLen = *cp2++;
  LONG ch1, ch2;

  while (hNmLen-- > 0 && GetCICh(cp1, ch1) == GetCICh(cp2, ch2))
    cp1++, cp2++;

  return hNmLen < 0 && *cp1 == ':' ? cp1 + 1 : FileSpec;
} /* GetCommandLine */


#define FAILATCMD        "FailAt 2147483647\n"
#define STRLEN_FAILATCMD 18

/*
 *  Run a command asynchronously.
 */

STATIC LONG
ExecCommand(char *CmdLine, struct MsgPort **ParentPort,
            struct Process *Requester, struct FileHandle *ChildIO,
            LONG IoDirection)
{
  char *commandLine;
  struct ChildMsg *childMsg;
  struct TagItem processTags[9];
  struct Process *child, *thisProcess = (struct Process *) FindTask(NULL);
  struct CommandLineInterface *cli = BADDR(Requester->pr_CLI);
  BPTR childOI;
  struct LocalVar *pVar;
  BPTR pathList, homeDir;
  LONG ioErr;

  /*
   *  Copy shell variables, aliases, and path from requester process.
   *  NOTE: This is not completely safe!
   */
  Forbid();
  for (pVar = (struct LocalVar *) Requester->pr_LocalVars.mlh_TailPred;
       pVar->lv_Node.ln_Pred;
       pVar = (struct LocalVar *) pVar->lv_Node.ln_Pred)
  {
    if (FindVar(pVar->lv_Node.ln_Name, pVar->lv_Node.ln_Type) == NULL)
      SetVar(pVar->lv_Node.ln_Name, pVar->lv_Value, pVar->lv_Len,
             pVar->lv_Flags | pVar->lv_Node.ln_Type & ~LVF_IGNORE);
  }

  pathList = ClonePathList(cli);

  Permit();

  if ((childOI = CloneProcessIO(Requester, IoDirection)) == NULL)
  {
    ioErr = IoErr();
    FreePathList(pathList);

    return ioErr;
  }

  if ((homeDir = DupLock(Requester->pr_CurrentDir)) == NULL)
  {
    ioErr = IoErr();
    Close(childOI);
    FreePathList(pathList);

    return ioErr;
  }

  /*
   *  Create parent's port.
   */
  if ((*ParentPort = CreateMsgPort()) == NULL)
  {
    UnLock(homeDir);
    Close(childOI);
    FreePathList(pathList);

    return ERROR_NO_FREE_STORE;
  }

  /*
   *  Build command line from argument vector.  First count the length.
   *
   *  We insert a `FailAt 2147483647' (0x7fffffff) so the shell doesn't
   *  print `Command foo failed' when we call System() to execute the
   *  command.
   */
  if ((childMsg = AllocVec(sizeof(struct ChildMsg) + strlen(CmdLine) +
                           STRLEN_FAILATCMD + 1, MEMF_PUBLIC)) == NULL)
  {
    DeleteMsgPort(*ParentPort);
    *ParentPort = NULL;
    UnLock(homeDir);
    Close(childOI);
    FreePathList(pathList);

    return ERROR_NO_FREE_STORE;
  }

  childMsg->ExecMsg.mn_Node.ln_Type = NT_MESSAGE;
  childMsg->ExecMsg.mn_Node.ln_Pri = 0;
  childMsg->ExecMsg.mn_ReplyPort = *ParentPort;
  childMsg->ExecMsg.mn_Length = sizeof(struct ChildMsg);
  childMsg->CmdLine = commandLine = (char *) (childMsg + 1);

  /* Setup the grandchild's [sic] stack */
  childMsg->StackSize = cli ? cli->cli_DefaultStack << 2 :
                              Requester->pr_StackSize;
  childMsg->PathList = pathList;
  childMsg->Flags = pathList ? CHMF_PATH : 0;
  childMsg->RC = 0;

  strcpy(commandLine, FAILATCMD);
  strcpy(commandLine + STRLEN_FAILATCMD, CmdLine);

  processTags[0].ti_Tag = NP_Entry;
  processTags[0].ti_Data = (Tag) _ChildProcess;
  processTags[1].ti_Tag = NP_Input;
  processTags[2].ti_Tag = NP_Output;
  processTags[3].ti_Tag = NP_StackSize;
  processTags[3].ti_Data = thisProcess->pr_StackSize;
  processTags[4].ti_Tag = NP_Cli;
  processTags[4].ti_Data = TRUE;
  processTags[5].ti_Tag = NP_Name;
  processTags[5].ti_Data = (Tag) "Kicker Process";
  processTags[6].ti_Tag = NP_Priority;
  processTags[6].ti_Data = Requester->pr_Task.tc_Node.ln_Pri;
  processTags[7].ti_Tag = NP_CurrentDir;
  processTags[7].ti_Data = homeDir;
  processTags[8].ti_Tag = TAG_DONE;

  if (IoDirection == PPIPE_READ)
  {
    processTags[1].ti_Data = childOI;
    processTags[2].ti_Data = MKBADDR(ChildIO);
  }
  else /* IoDirection == PPIPE_WRITE */
  {
    processTags[1].ti_Data = MKBADDR(ChildIO);
    processTags[2].ti_Data = childOI;
  }

  /*
   *  Start our `kicker' process.  This process consists of the _ChildProcess()
   *  function above.  The _ChildProcess() function then executes the command.
   */
  if ((child = CreateNewProc(processTags)) == NULL)
  {
    FreeVec(childMsg);
    DeleteMsgPort(*ParentPort);
    *ParentPort = NULL;
    UnLock(homeDir);
    Close(childOI);
    FreePathList(pathList);

    return ERROR_NO_FREE_STORE;
  }

  /* now pass the child the startup message */
  PutMsg(&child->pr_MsgPort, (struct Message *) childMsg);

  return 0;
} /* ExecCommand */


STATIC BPTR
CloneProcessIO(struct Process *Friend, LONG IoDirection)
{
  BPTR origFH;
  BPTR lock;
  BPTR fh = NULL;
  LONG pos = -1;

  if (IoDirection == PPIPE_READ)
    origFH = Friend->pr_CIS;
  else
    origFH = Friend->pr_COS;

  if (origFH)
  {
    pos = Seek(origFH, 0, OFFSET_CURRENT);
    if (IoErr()) /* Handle V36/37 Seek() bug.  */
      pos = -1;
  }

  if ((lock = DupLockFromFH(origFH)) == NULL ||
      (fh = OpenFromLock(lock)) == NULL)
  {
    if (lock)
      UnLock(lock);
    fh = Open("CONSOLE:", MODE_OLDFILE);
  }
  else if (fh && pos > 0)
    Seek(fh, pos, OFFSET_BEGINNING);

  return fh;
} /* CloneProcessIO */


/*
 *  NOTE: This routine should be called from within a Forbid();
 *        but it is still unsafe due to the call to DupLock().
 */
STATIC BPTR
ClonePathList(struct CommandLineInterface *Peer)
{
  BPTR pathList = NULL;
  struct PathEntry *pathEntry, *pathET;

  if (Peer)
  {
    for (pathEntry = BADDR(Peer->cli_CommandDir),
         pathET = (struct PathEntry *) &pathList;
         pathEntry;
         pathEntry = BADDR(pathEntry->pe_NextPathEntry))
    {
      struct PathEntry *newPE;

      if (newPE = AllocMem(sizeof(struct PathEntry), MEMF_PUBLIC))
      {
        newPE->pe_NextPathEntry = NULL;
        newPE->pe_PathLock = DupLock(pathEntry->pe_PathLock);
        pathET->pe_NextPathEntry = MKBADDR(newPE);
        pathET = newPE;
      }
      else
        break;
    }
  }

  return pathList;
} /* ClonePathList */


STATIC void
FreePathList(BPTR PathList)
{
  struct PathEntry *pathEntry;

  while (pathEntry = BADDR(PathList))
  {
    PathList = pathEntry->pe_NextPathEntry;
    UnLock(pathEntry->pe_PathLock);
    FreeMem(pathEntry, sizeof(struct PathEntry));
  }
} /* FreePathList */
