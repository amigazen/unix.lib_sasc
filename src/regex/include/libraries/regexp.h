#ifndef LIBRARIES_REGEXP_H
#define LIBRARIES_REGEXP_H
/*
**	$VER: regexp.h 38.1 (26.2.98)
**
**	(C) Copyright 1998 Matthias Bethke
*/

#ifndef DOS_DOS_H
#include "dos/dos.h"
#endif

/*
** Error numbers returned by IoErr() if a regexp.library function
** fails. Use RegXlatError() to obtain the error text for a code
*/

#define REXPERR_NULLARG -1						/* "NULL argument" */
#define REXPERR_REXPTOOBIG -2					/* "Regexp too big */
#define REXPERR_TOOMANYPARENS -3				/* "Too many parentheses" */
#define REXPERR_UNMATCHEDPARENS -4			/* "unmatched parentheses */
#define REXPERR_JUNKONEND -5					/* "Junk on end" (should never happen) */
#define REXPERR_REPKLEENECBE -6				/* "Repetition/Kleene (+*) could be empty" */
#define REXPERR_NESTEDPOSTFIXOP -7			/* "Nested postfix (+*?) operators" */
#define REXPERR_INVALIDRANGE -8				/* "Invalid range" */
#define REXPERR_UNMATCHEDRNGBRKT -9			/* "Unmatched range bracket" */
#define REXPERR_INTERNALURP -10				/* "Internal urp" */
#define REXPERR_PFOPFOLLOWSNOTHING -11		/* "Postfix operator (+*?) follows nothing" */
#define REXPERR_TRAILINGBKSLASH -12			/* "Trailing backslash" */
#define REXPERR_CORRUPTEDPROG -13			/* "Corrupted program" */
#define REXPERR_CORRUPTEDMEM -14				/* "Corrupted memory" */
#define REXPERR_CORRUPTEDPTRS -15			/* "Corrupted pointers" */
#define REXPERR_INTERNALFOULUP -16			/* "Internal foulup" */


/*
** better don't mess with struct regexp, use it as an abstract
** handle for expressions only!
** You may read startp/endp pointers to get start/end of matched
** (sub)expressions though.
*/

#define NSUBEXP  10
typedef struct regexp {
	STRPTR startp[NSUBEXP];
	STRPTR endp[NSUBEXP];
	UBYTE regstart;		/* Internal use only. */
	UBYTE reganch;			/* Internal use only. */
	STRPTR regmust;		/* Internal use only. */
	LONG regmlen;			/* Internal use only. */
	UBYTE program[1];		/* Unwarranted chumminess with compiler. */
} regexp;


#endif /* LIBRARIES_REGEXP_H */
