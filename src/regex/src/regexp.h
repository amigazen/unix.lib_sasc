/*
 * Definitions etc. for regexp(3) routines.
 *
 * Caveat:  this is V8 regexp(3) [actually, a reimplementation thereof],
 * not the System V one.
 *
 *	$NetBSD: regexp.h,v 1.3 1997/10/09 10:21:21 lukem Exp $
 */

/* Amiga prototypes for exported library functions */
__saveds __asm regexp *RegComp(register __a0 STRPTR);
__saveds __asm void RegFree(register __a1 regexp*);
__saveds __asm LONG RegExec(register __a0 regexp*, register __a1 STRPTR);
/* Amiga version only */
__saveds __asm STRPTR RegXlatError(register __d0 LONG);
