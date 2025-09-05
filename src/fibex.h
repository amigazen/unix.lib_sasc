/*
 * fibex.h - extensions to FIB (File Information Block)
 *
 * This file defines the extended FileInfoBlock structure that includes
 * OwnerUID and OwnerGID fields for proper file ownership support.
 * Based on AmigaOS 3.0+ filesystem extensions.
 */

#ifndef _FIBEX_H
#define _FIBEX_H

#include <dos/dosextens.h>

/*
 * This is already in 3.0 includes, but we define it here for compatibility
 */
#ifndef FIBB_OTR_READ
#define FileInfoBlock myFileInfoBlock

/* Returned by Examine() and ExNext(), must be on a LONG boundary */
struct FileInfoBlock {
   LONG	  fib_DiskKey;
   LONG	  fib_DirEntryType;
   char	  fib_FileName[108]; 
   LONG	  fib_Protection;    
   LONG	  fib_EntryType;
   LONG	  fib_Size;	     
   LONG	  fib_NumBlocks;     
   struct DateStamp fib_Date;
   char	  fib_Comment[80];   
   /* Note: the following fields are not supported by all filesystems.	*/
   /* They should be initialized to 0 sending an ACTION_EXAMINE packet.	*/
   /* When Examine() is called, these are set to 0 for you.		*/
   /* AllocDosObject() also initializes them to 0.			*/
   UWORD  fib_OwnerUID;		/* owner's UID */
   UWORD  fib_OwnerGID;		/* owner's GID */
   char	  fib_Reserved[32];
}; /* FileInfoBlock */

/* FIB stands for FileInfoBlock */

/* FIBB are bit definitions, FIBF are field definitions */
/* Regular RWED bits are 0 == allowed. */
/* NOTE: GRP and OTR RWED permissions are 0 == not allowed! */
/* Group and Other permissions are not directly handled by the filesystem */
#define FIBB_OTR_READ	   15	/* Other: file is readable */
#define FIBB_OTR_WRITE	   14	/* Other: file is writable */
#define FIBB_OTR_EXECUTE   13	/* Other: file is executable */
#define FIBB_OTR_DELETE    12	/* Other: prevent file from being deleted */
#define FIBB_GRP_READ	   11	/* Group: file is readable */
#define FIBB_GRP_WRITE	   10	/* Group: file is writable */
#define FIBB_GRP_EXECUTE   9	/* Group: file is executable */
#define FIBB_GRP_DELETE    8	/* Group: file is writable */

#define FIBF_OTR_READ	   (1<<FIBB_OTR_READ)
#define FIBF_OTR_WRITE	   (1<<FIBB_OTR_WRITE)
#define FIBF_OTR_EXECUTE   (1<<FIBB_OTR_EXECUTE)
#define FIBF_OTR_DELETE    (1<<FIBB_OTR_DELETE)
#define FIBF_GRP_READ	   (1<<FIBB_GRP_READ)
#define FIBF_GRP_WRITE	   (1<<FIBB_GRP_WRITE)
#define FIBF_GRP_EXECUTE   (1<<FIBB_GRP_EXECUTE)
#define FIBF_GRP_DELETE    (1<<FIBB_GRP_DELETE)

#endif /* defined(FIBB_OTR_READ) */

#endif /* _FIBEX_H */
