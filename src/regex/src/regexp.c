/*
 * regcomp and regexec -- regsub and regerror are elsewhere
 *
 *	Copyright (c) 1986 by University of Toronto.
 *	Written by Henry Spencer.  Not derived from licensed software.
 *
 *	Permission is granted to anyone to use this software for any
 *	purpose on any computer system, and to redistribute it freely,
 *	subject to the following restrictions:
 *
 *	1. The author is not responsible for the consequences of use of
 *		this software, no matter how awful, even if they arise
 *		from defects in it.
 *
 *	2. The origin of this software must not be misrepresented, either
 *		by explicit claim or by omission.
 *
 *	3. Altered versions must be plainly marked as such, and must not
 *		be misrepresented as being the original software.
 *** THIS IS AN ALTERED VERSION.  It was altered by John Gilmore,
 *** hoptoad!gnu, on 27 Dec 1986, to add \n as an alternative to |
 *** to assist in implementing egrep.
 *** THIS IS AN ALTERED VERSION.  It was altered by John Gilmore,
 *** hoptoad!gnu, on 27 Dec 1986, to add \< and \> for word-matching
 *** as in BSD grep and ex.
 *** THIS IS AN ALTERED VERSION.  It was altered by John Gilmore,
 *** hoptoad!gnu, on 28 Dec 1986, to optimize characters quoted with \.
 *** THIS IS AN ALTERED VERSION.  It was altered by James A. Woods,
 *** ames!jaw, on 19 June 1987, to quash a regcomp() redundancy.
 *** THIS IS AN ALTERED VERSION.  It was altered by Matthias Bethke,
 *** Matthias.Bethke@stud.uni-erlangen.de, on 10-Feb-98
 * Main changes for Amiga shared library use:
 * - no stdio/stdlib code any more
 * - global data eliminated to make code reentrant
 * - this also meant changing almost all of the subroutine protos
 * - explicit register definitions for SAS/C added to RegComp/RegExec
 *   (also changed spelling and types to be more Amigaish :))
 *
 * Beware that some of this code is subtly aware of the way operator
 * precedence is structured in regular expressions.  Serious changes in
 * regular-expression syntax might require a total rethink.
 * 
 */

#include <ctype.h>
#include <string.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <exec/memory.h>
#include <libraries/regexp.h>
#include "regmagic.h"
#include "regexp.h"
#include "regexpbase.h"

#define SysBase (RegexpBase->reb_SysBase)
#define DOSBase (RegexpBase->reb_DOSBase)
extern struct RegexpBase *RegexpBase;

/*
 * The "internal use only" fields in regexp.h are present to pass info from
 * compile to execute that permits the execute phase to run lots faster on
 * simple cases.  They are:
 *
 * regstart	char that must begin a match; '\0' if none obvious
 * reganch	is the match anchored (at beginning-of-line only)?
 * regmust	string (pointer into program) that match must include, or NULL
 * regmlen	length of regmust string
 *
 * Regstart and reganch permit very fast decisions on suitable starting points
 * for a match, cutting down the work a lot.  Regmust permits fast rejection
 * of lines that cannot possibly match.  The regmust tests are costly enough
 * that regcomp() supplies a regmust only if the r.e. contains something
 * potentially expensive (at present, the only such thing detected is * or +
 * at the start of the r.e., which can involve a lot of backup).  Regmlen is
 * supplied because the test in regexec() needs it and regcomp() is computing
 * it anyway.
 */

/*
 * Structure for regexp "program".  This is essentially a linear encoding
 * of a nondeterministic finite-state machine (aka syntax charts or
 * "railroad normal form" in parsing technology).  Each node is an opcode
 * plus a "next" pointer, possibly plus an operand.  "Next" pointers of
 * all nodes except BRANCH implement concatenation; a "next" pointer with
 * a BRANCH on both ends of it is connecting two alternatives.  (Here we
 * have one of the subtle syntax dependencies:  an individual BRANCH (as
 * opposed to a collection of them) is never concatenated with anything
 * because of operator precedence.)  The operand of some types of node is
 * a literal string; for others, it is a node leading into a sub-FSM.  In
 * particular, the operand of a BRANCH node is the first node of the branch.
 * (NB this is *not* a tree structure:  the tail of the branch connects
 * to the thing following the set of BRANCHes.)  The opcodes are:
 */

/* definition	number	opnd?	meaning */
#define	END	0	/* no	End of program. */
#define	BOL	1	/* no	Match "" at beginning of line. */
#define	EOL	2	/* no	Match "" at end of line. */
#define	ANY	3	/* no	Match any one character. */
#define	ANYOF	4	/* str	Match any character in this string. */
#define	ANYBUT	5	/* str	Match any character not in this string. */
#define	BRANCH	6	/* node	Match this alternative, or the next... */
#define	BACK	7	/* no	Match "", "next" ptr points backward. */
#define	EXACTLY	8	/* str	Match this string. */
#define	NOTHING	9	/* no	Match empty string. */
#define	STAR	10	/* node	Match this (simple) thing 0 or more times. */
#define	PLUS	11	/* node	Match this (simple) thing 1 or more times. */
#define	WORDA	12	/* no	Match "" at wordchar, where prev is nonword */
#define	WORDZ	13	/* no	Match "" at nonwordchar, where prev is word */
#define	OPEN	20	/* no	Mark this point in input as start of #n. */
			/*	OPEN+1 is number 1, etc. */
#define	CLOSE	30	/* no	Analogous to OPEN. */


/*
 * Opcode notes:
 *
 * BRANCH	The set of branches constituting a single choice are hooked
 *		together with their "next" pointers, since precedence prevents
 *		anything being concatenated to any individual branch.  The
 *		"next" pointer of the last BRANCH in a choice points to the
 *		thing following the whole choice.  This is also where the
 *		final "next" pointer of each individual branch points; each
 *		branch starts with the operand node of a BRANCH node.
 *
 * BACK		Normal "next" pointers all implicitly point forward; BACK
 *		exists to make loop structures possible.
 *
 * STAR,PLUS	'?', and complex '*' and '+', are implemented as circular
 *		BRANCH structures using BACK.  Simple cases (one character
 *		per match) are implemented with STAR and PLUS for speed
 *		and to minimize recursive plunges.
 *
 * OPEN,CLOSE	...are numbered at compile time.
 */

/*
 * A node is one char of opcode followed by two chars of "next" pointer.
 * "Next" pointers are stored as two 8-bit pieces, high order first.  The
 * value is a positive offset from the opcode of the node containing it.
 * An operand, if any, simply follows the node.  (Note that much of the
 * code generation knows about this implicit relationship.)
 *
 * Using two bytes for the "next" pointer is vast overkill for most things,
 * but allows patterns to get big without disasters.
 */
#define	OP(p)	(*(p))
#define	NEXT(p)	(((*((p)+1)&0377)<<8) + (*((p)+2)&0377))
#define	OPERAND(p)	((p) + 3)

/*
 * See regmagic.h for one further detail of program structure.
 */


/*
 * Utility definitions.
 */
#ifndef CHARBITS
#define	UCHARAT(p)	((LONG)*(STRPTR)(p))
#else
#define	UCHARAT(p)	((LONG)*(p)&CHARBITS)
#endif

#define	FAIL(m,r)	{ SetIoErr(m); return(r); }
#define	ISMULT(c)	((c) == '*' || (c) == '+' || (c) == '?')

/*
 * Flags to be passed up and down.
 */
#define	HASWIDTH	01	/* Known never to match null string. */
#define	SIMPLE		02	/* Simple enough to be STAR/PLUS operand. */
#define	SPSTART		04	/* Starts with * or +. */
#define	WORST		0	/* Worst case. */


/*
** the following tw typedefs are required to get rid of the global
** data that the *IX version contains but which make the code non-reetrant
*/
typedef struct RegcompGlobals
{
	STRPTR regparse;	/* Input-scan pointer. */
	LONG regnpar;		/* () count. */
	STRPTR regcode;	/* Code-emit pointer; &regdummy = don't. */
	LONG regsize;		/* Code size. */
} rc_globals;

typedef struct RegexecGlobals
{
	STRPTR reginput;		/* String-input pointer. */
	STRPTR regbol;			/* Beginning of input, for ^ check. */
	STRPTR *regstartp;	/* Pointer to startp array. */
	STRPTR *regendp;		/* Ditto for endp. */
} re_globals;

static UBYTE regdummy;	// yes, static data! (not modified, just to take
								// the address off

/*
 * Forward declarations for regcomp()'s friends.
 */
#ifndef STATIC
#define	STATIC	static
#endif
STATIC UBYTE *reg (rc_globals *,LONG, LONG *);
STATIC UBYTE *regbranch (rc_globals *,LONG *);
STATIC UBYTE *regpiece (rc_globals *,LONG *);
STATIC UBYTE *regatom (rc_globals *,LONG *);
STATIC UBYTE *regnode (rc_globals *,UBYTE);
STATIC UBYTE *regnext (STRPTR);
STATIC void regc (rc_globals *,UBYTE);
STATIC void reginsert (rc_globals *,UBYTE, STRPTR);
STATIC void regtail (rc_globals *,STRPTR, STRPTR);
STATIC void regoptail (rc_globals *,STRPTR, STRPTR);



/* added for Amiga convenience */
__saveds __asm void RegFree(register __a1 regexp *re) { FreeVec(re); }

/*
 - regcomp - compile a regular expression into internal code
 *
 * We can't allocate space until we know how big the compiled form will be,
 * but we can't compile it (and thus know how big it is) until we've got a
 * place to put the code.  So we cheat:  we compile it twice, once with code
 * generation turned off and size counting turned on, and once "for real".
 * This also means that we don't allocate space until we are sure that the
 * thing really will compile successfully, and we never have to move the
 * code and thus invalidate pointers into it.  (Note that it has to be in
 * one piece because free() must be able to free it all.)
 *
 * Beware that the optimization-preparation code in here knows about some
 * of the structure of the compiled regexp.
 */
__saveds __asm regexp *RegComp(register __a0 STRPTR exp)
{
rc_globals global_data;
register rc_globals *_rcglobs;
register regexp *r;
register STRPTR scan, longest;
register LONG len;
LONG flags;


	_rcglobs = &global_data;


	if (exp == NULL)
		FAIL(REXPERR_NULLARG,NULL);

	/* First pass: determine size, legality. */
#ifdef notdef
	if (exp[0] == '.' && exp[1] == '*') exp += 2;  /* aid grep */
#endif
	(_rcglobs->regparse) = (STRPTR)exp;
	(_rcglobs->regnpar) = 1;
	(_rcglobs->regsize) = 0L;
	(_rcglobs->regcode) = &regdummy;
	regc(_rcglobs, (UBYTE)MAGIC);
	if (reg(_rcglobs,0, &flags) == NULL)
		return(NULL);

	/* Small enough for pointer-storage convention? */
	if ((_rcglobs->regsize) >= 32767L)		/* Probably could be 65535L. */
		FAIL(REXPERR_REXPTOOBIG,NULL);

	/* Allocate space. */
	r = (regexp *)AllocVec(sizeof(regexp) + (unsigned)(_rcglobs->regsize),MEMF_ANY);
	if (r == NULL)
		FAIL(ERROR_NO_FREE_STORE,NULL);

	/* Second pass: emit code. */
	(_rcglobs->regparse) = (STRPTR)exp;
	(_rcglobs->regnpar) = 1;
	(_rcglobs->regcode) = r->program;
	regc(_rcglobs, MAGIC);
	if (reg(_rcglobs,0, &flags) == NULL)
		return(NULL);

	/* Dig out information for optimizations. */
	r->regstart = '\0';	/* Worst-case defaults. */
	r->reganch = 0;
	r->regmust = NULL;
	r->regmlen = 0;
	scan = r->program+1;			/* First BRANCH. */
	if (OP(regnext(scan)) == END) {		/* Only one top-level choice. */
		scan = OPERAND(scan);

		/* Starting-point info. */
		if (OP(scan) == EXACTLY)
			r->regstart = *OPERAND(scan);
		else if (OP(scan) == BOL)
			r->reganch++;

		/*
		 * If there's something expensive in the r.e., find the
		 * longest literal string that must appear and make it the
		 * regmust.  Resolve ties in favor of later strings, since
		 * the regstart check works with the beginning of the r.e.
		 * and avoiding duplication strengthens checking.  Not a
		 * strong reason, but sufficient in the absence of others.
		 */
		if (flags&SPSTART) {
			longest = NULL;
			len = 0;
			for (; scan != NULL; scan = regnext(scan))
				if (OP(scan) == EXACTLY && strlen(OPERAND(scan)) >= len) {
					longest = OPERAND(scan);
					len = strlen(OPERAND(scan));
				}
			r->regmust = longest;
			r->regmlen = len;
		}
	}

	return(r);
}

/*
 - reg - regular expression, i.e. main body or parenthesized thing
 *
 * Caller must absorb opening parenthesis.
 *
 * Combining parenthesis handling with the base level of regular expression
 * is a trifle forced, but the need to tie the tails of the branches to what
 * follows makes it hard to avoid.
 */
static UBYTE *reg(rc_globals *_rcglobs,LONG paren, LONG *flagp)
{
register STRPTR ret,br,ender;
register LONG parno = 0;
LONG flags;

	*flagp = HASWIDTH;	/* Tentatively. */

	/* Make an OPEN node, if parenthesized. */
	if (paren) {
		if ((_rcglobs->regnpar) >= NSUBEXP)
			FAIL(REXPERR_TOOMANYPARENS,NULL);
		parno = (_rcglobs->regnpar);
		(_rcglobs->regnpar)++;
		ret = regnode(_rcglobs, OPEN+parno);
	} else
		ret = NULL;

	/* Pick up the branches, linking them together. */
	br = regbranch(_rcglobs, &flags);
	if (br == NULL)
		return(NULL);
	if (ret != NULL)
		regtail(_rcglobs, ret, br);	/* OPEN -> first. */
	else
		ret = br;
	if (!(flags&HASWIDTH))
		*flagp &= ~HASWIDTH;
	*flagp |= flags&SPSTART;
	while (*(_rcglobs->regparse) == '|' || *(_rcglobs->regparse) == '\n') {
		(_rcglobs->regparse)++;
		br = regbranch(_rcglobs, &flags);
		if (br == NULL)
			return(NULL);
		regtail(_rcglobs, ret, br);	/* BRANCH -> BRANCH. */
		if (!(flags&HASWIDTH))
			*flagp &= ~HASWIDTH;
		*flagp |= flags&SPSTART;
	}

	/* Make a closing node, and hook it on the end. */
	ender = regnode(_rcglobs, (paren) ? CLOSE+parno : END);	
	regtail(_rcglobs, ret, ender);

	/* Hook the tails of the branches to the closing node. */
	for (br = ret; br != NULL; br = regnext(br))
		regoptail(_rcglobs, br, ender);

	/* Check for proper termination. */
	if (paren && *(_rcglobs->regparse)++ != ')') {
		FAIL(REXPERR_UNMATCHEDPARENS,NULL);
	} else if (!paren && *(_rcglobs->regparse) != '\0') {
		if (*(_rcglobs->regparse) == ')') {
			FAIL(REXPERR_UNMATCHEDPARENS,NULL);
		} else
			FAIL(REXPERR_JUNKONEND,NULL);	/* "Can't happen". */
		/* NOTREACHED */
	}

	return(ret);
}

/*
 - regbranch - one alternative of an | operator
 *
 * Implements the concatenation operator.
 */
static UBYTE *regbranch(rc_globals *_rcglobs, LONG *flagp)
{
register STRPTR ret,chain,latest;
LONG flags;

	*flagp = WORST;		/* Tentatively. */

	ret = regnode(_rcglobs, BRANCH);
	chain = NULL;
	while (*(_rcglobs->regparse) != '\0' && *(_rcglobs->regparse) != ')' &&
	       *(_rcglobs->regparse) != '\n' && *(_rcglobs->regparse) != '|') {
		latest = regpiece(_rcglobs, &flags);
		if (latest == NULL)
			return(NULL);
		*flagp |= flags&HASWIDTH;
		if (chain == NULL)	/* First piece. */
			*flagp |= flags&SPSTART;
		else
			regtail(_rcglobs, chain, latest);
		chain = latest;
	}
	if (chain == NULL)	/* Loop ran zero times. */
		(void) regnode(_rcglobs, NOTHING);

	return(ret);
}

/*
 - regpiece - something followed by possible [*+?]
 *
 * Note that the branching code sequences used for ? and the general cases
 * of * and + are somewhat optimized:  they use the same NOTHING node as
 * both the endmarker for their branch list and the body of the last branch.
 * It might seem that this node could be dispensed with entirely, but the
 * endmarker role is not redundant.
 */
static UBYTE *regpiece(rc_globals *_rcglobs, LONG *flagp)
{
register STRPTR ret,next;
register UBYTE op;
LONG flags;

	ret = regatom(_rcglobs, &flags);
	if (ret == NULL)
		return(NULL);

	op = *(_rcglobs->regparse);
	if (!ISMULT(op)) {
		*flagp = flags;
		return(ret);
	}

	if (!(flags&HASWIDTH) && op != '?')
		FAIL(REXPERR_REPKLEENECBE,NULL);
	*flagp = (op != '+') ? (WORST|SPSTART) : (WORST|HASWIDTH);

	if (op == '*' && (flags&SIMPLE))
		reginsert(_rcglobs, STAR, ret);
	else if (op == '*') {
		/* Emit x* as (x&|), where & means "self". */
		reginsert(_rcglobs, BRANCH, ret);			/* Either x */
		regoptail(_rcglobs, ret, regnode(_rcglobs, BACK));		/* and loop */
		regoptail(_rcglobs, ret, ret);			/* back */
		regtail(_rcglobs, ret, regnode(_rcglobs, BRANCH));		/* or */
		regtail(_rcglobs, ret, regnode(_rcglobs, NOTHING));		/* null. */
	} else if (op == '+' && (flags&SIMPLE))
		reginsert(_rcglobs, PLUS, ret);
	else if (op == '+') {
		/* Emit x+ as x(&|), where & means "self". */
		next = regnode(_rcglobs, BRANCH);			/* Either */
		regtail(_rcglobs, ret, next);
		regtail(_rcglobs, regnode(_rcglobs, BACK), ret);		/* loop back */
		regtail(_rcglobs, next, regnode(_rcglobs, BRANCH));		/* or */
		regtail(_rcglobs, ret, regnode(_rcglobs, NOTHING));		/* null. */
	} else if (op == '?') {
		/* Emit x? as (x|) */
		reginsert(_rcglobs, BRANCH, ret);			/* Either x */
		regtail(_rcglobs, ret, regnode(_rcglobs, BRANCH));		/* or */
		next = regnode(_rcglobs, NOTHING);		/* null. */
		regtail(_rcglobs, ret, next);
		regoptail(_rcglobs, ret, next);
	}
	(_rcglobs->regparse)++;
	if (ISMULT(*(_rcglobs->regparse)))
		FAIL(REXPERR_NESTEDPOSTFIXOP,NULL);

	return(ret);
}

/*
 - regatom - the lowest level
 *
 * Optimization:  gobbles an entire sequence of ordinary characters so that
 * it can turn them into a single node, which is smaller to store and
 * faster to run.  Backslashed characters are exceptions, each becoming a
 * separate node; the code is simpler that way and it's not worth fixing.
 */
static UBYTE *regatom(rc_globals *_rcglobs,LONG *flagp)
{
register STRPTR ret;
LONG flags;

	*flagp = WORST;		/* Tentatively. */

	switch (*(_rcglobs->regparse)++) {
	/* FIXME: these chars only have meaning at beg/end of pat? */
	case '^':
		ret = regnode(_rcglobs, BOL);
		break;
	case '$':
		ret = regnode(_rcglobs, EOL);
		break;
	case '.':
		ret = regnode(_rcglobs, ANY);
		*flagp |= HASWIDTH|SIMPLE;
		break;
	case '[': {
			register LONG class;
			register LONG classend;

			if (*(_rcglobs->regparse) == '^') {	/* Complement of range. */
				ret = regnode(_rcglobs, ANYBUT);
				(_rcglobs->regparse)++;
			} else
				ret = regnode(_rcglobs, ANYOF);
			if (*(_rcglobs->regparse) == ']' || *(_rcglobs->regparse) == '-')
				regc(_rcglobs, *(_rcglobs->regparse)++);
			while (*(_rcglobs->regparse) != '\0' && *(_rcglobs->regparse) != ']') {
				if (*(_rcglobs->regparse) == '-') {
					(_rcglobs->regparse)++;
					if (*(_rcglobs->regparse) == ']' || *(_rcglobs->regparse) == '\0')
						regc(_rcglobs, '-');
					else {
						class = UCHARAT((_rcglobs->regparse)-2)+1;
						classend = UCHARAT((_rcglobs->regparse));
						if (class > classend+1)
							FAIL(REXPERR_INVALIDRANGE,NULL);
						for (; class <= classend; class++)
							regc(_rcglobs, class);
						(_rcglobs->regparse)++;
					}
				} else
					regc(_rcglobs, *(_rcglobs->regparse)++);
			}
			regc(_rcglobs, '\0');
			if (*(_rcglobs->regparse) != ']')
				FAIL(REXPERR_UNMATCHEDRNGBRKT,NULL);
			(_rcglobs->regparse)++;
			*flagp |= HASWIDTH|SIMPLE;
		}
		break;
	case '(':
		ret = reg(_rcglobs,1, &flags);
		if (ret == NULL)
			return(NULL);
		*flagp |= flags&(HASWIDTH|SPSTART);
		break;
	case '\0':
	case '|':
	case '\n':
	case ')':
		FAIL(REXPERR_INTERNALURP,NULL);	/* Supposed to be caught earlier. */
		break;
	case '?':
	case '+':
	case '*':
		FAIL(REXPERR_PFOPFOLLOWSNOTHING,NULL);
		break;
	case '\\':
		switch (*(_rcglobs->regparse)++) {
		case '\0':
			FAIL(REXPERR_TRAILINGBKSLASH,NULL);
			break;
		case '<':
			ret = regnode(_rcglobs, WORDA);
			break;
		case '>':
			ret = regnode(_rcglobs, WORDZ);
			break;
		/* FIXME: Someday handle \1, \2, ... */
		default:
			/* Handle general quoted chars in exact-match routine */
			goto de_fault;
		}
		break;
	de_fault:
	default:
		/*
		 * Encode a string of characters to be matched exactly.
		 *
		 * This is a bit tricky due to quoted chars and due to
		 * '*', '+', and '?' taking the SINGLE char previous
		 * as their operand.
		 *
		 * On entry, the char at (_rcglobs->regparse)[-1] is going to go
		 * into the string, no matter what it is.  (It could be
		 * following a \ if we are entered from the '\' case.)
		 * 
		 * Basic idea is to pick up a good char in  ch  and
		 * examine the next char.  If it's *+? then we twiddle.
		 * If it's \ then we frozzle.  If it's other magic char
		 * we push  ch  and terminate the string.  If none of the
		 * above, we push  ch  on the string and go around again.
		 *
		 *  regprev  is used to remember where "the current char"
		 * starts in the string, if due to a *+? we need to back
		 * up and put the current char in a separate, 1-char, string.
		 * When  regprev  is NULL,  ch  is the only char in the
		 * string; this is used in *+? handling, and in setting
		 * flags |= SIMPLE at the end.
		 */
		{
			UBYTE *regprev;
			register UBYTE ch;

			(_rcglobs->regparse)--;			/* Look at cur char */
			ret = regnode(_rcglobs, EXACTLY);
			for ( regprev = 0 ; ; ) {
				ch = *(_rcglobs->regparse)++;	/* Get current char */
				switch (*(_rcglobs->regparse)) {	/* look at next one */

				default:
					regc(_rcglobs, ch);	/* Add cur to string */
					break;

				case '.': case '[': case '(':
				case ')': case '|': case '\n':
				case '$': case '^':
				case '\0':
				/* FIXME, $ and ^ should not always be magic */
				magic:
					regc(_rcglobs, ch);	/* dump cur char */
					goto done;	/* and we are done */

				case '?': case '+': case '*':
					if (!regprev) 	/* If just ch in str, */
						goto magic;	/* use it */
					/* End mult-char string one early */
					(_rcglobs->regparse) = regprev; /* Back up parse */
					goto done;

				case '\\':
					regc(_rcglobs, ch);	/* Cur char OK */
					switch ((_rcglobs->regparse)[1]){ /* Look after \ */
					case '\0':
					case '<':
					case '>':
					/* FIXME: Someday handle \1, \2, ... */
						goto done; /* Not quoted */
					default:
						/* Backup point is \, scan							 * point is after it. */
						regprev = (_rcglobs->regparse);
						(_rcglobs->regparse)++; 
						continue;	/* NOT break; */
					}
				}
				regprev = (_rcglobs->regparse);	/* Set backup point */
			}
		done:
			regc(_rcglobs, '\0');
			*flagp |= HASWIDTH;
			if (!regprev)		/* One char? */
				*flagp |= SIMPLE;
		}
		break;
	}

	return(ret);
}

/*
 - regnode - emit a node
 */
static STRPTR regnode(rc_globals *_rcglobs, UBYTE op)
{
	register UBYTE *ret;
	register UBYTE *ptr;

	ret = (_rcglobs->regcode);
	if (ret == &regdummy) {
		(_rcglobs->regsize) += 3;
		return(ret);
	}

	ptr = ret;
	*ptr++ = op;
	*ptr++ = '\0';		/* Null "next" pointer. */
	*ptr++ = '\0';
	(_rcglobs->regcode) = ptr;

	return(ret);
}

/*
 - regc - emit (if appropriate) a byte of code
 */
static void regc(rc_globals *_rcglobs,UBYTE b)
{
	if ((_rcglobs->regcode) != &regdummy)
		*(_rcglobs->regcode)++ = b;
	else
		(_rcglobs->regsize)++;
}

/*
 - reginsert - insert an operator in front of already-emitted operand
 *
 * Means relocating the operand.
 */
static void reginsert(rc_globals *_rcglobs,UBYTE op, STRPTR opnd)
{
register STRPTR src,dst,place;

	if ((_rcglobs->regcode) == &regdummy) {
		(_rcglobs->regsize) += 3;
		return;
	}

	src = (_rcglobs->regcode);
	(_rcglobs->regcode) += 3;
	dst = (_rcglobs->regcode);
	while (src > opnd)
		*--dst = *--src;

	place = opnd;		/* Op node, where operand used to be. */
	*place++ = op;
	*place++ = '\0';
	*place++ = '\0';
}

/*
 - regtail - set the next-pointer at the end of a node chain
 */
static void regtail(rc_globals *_rcglobs,STRPTR p, STRPTR val)
{
register STRPTR scan,temp;
register LONG offset;

	if (p == &regdummy)
		return;

	/* Find last node. */
	scan = p;
	for (;;) {
		temp = regnext(scan);
		if (temp == NULL)
			break;
		scan = temp;
	}

	if (OP(scan) == BACK)
		offset = scan - val;
	else
		offset = val - scan;
	*(scan+1) = (offset>>8)&0377;
	*(scan+2) = offset&0377;
}

/*
 - regoptail - regtail on operand of first argument; nop if operandless
 */
static void regoptail(rc_globals *_rcglobs,STRPTR p, STRPTR val)
{
	/* "Operandless" and "op != BRANCH" are synonymous in practice. */
	if (p == NULL || p == &regdummy || OP(p) != BRANCH)
		return;
	regtail(_rcglobs, OPERAND(p), val);
}

/*
 * regexec and friends
 */

/*
 * Forwards.
 */
STATIC BOOL regtry (re_globals *, const regexp *, STRPTR);
STATIC BOOL regmatch (re_globals *, STRPTR);
STATIC LONG regrepeat (re_globals *, STRPTR);

#ifdef DEBUG
LONG regnarrate = 0;
void regdump ((regexp *));
STATIC UBYTE *regprop ((STRPTR));
#endif

/*
 - regexec - match a regexp against a string
 */
__saveds __asm LONG RegExec(register __a0 regexp *prog, register __a1 STRPTR string)
{
register STRPTR s;
re_globals global_data;
register re_globals *_reglobs;


	_reglobs = &global_data;


	/* Be paranoid... */
	if (prog == NULL || string == NULL) {
		FAIL(REXPERR_NULLARG,-1);
	}

	/* Check validity of program. */
	if (UCHARAT(prog->program) != MAGIC) {
		FAIL(REXPERR_CORRUPTEDPROG,-1);
	}

	/* If there is a "must appear" string, look for it. */
	if (prog->regmust != NULL) {
		s = (STRPTR)string;
		while ((s = strchr(s, prog->regmust[0])) != NULL) {
			if (strncmp(s, prog->regmust, prog->regmlen) == 0)
				break;	/* Found it. */
			s++;
		}
		if (s == NULL)	/* Not present. */
			return 0L;
	}

	/* Mark beginning of line for ^ . */
	(_reglobs->regbol) = (STRPTR)string;

	/* Simplest case:  anchored match need be tried only once. */
	if (prog->reganch)
		return(regtry(_reglobs, prog, string));

	/* Messy cases:  unanchored match. */
	s = (STRPTR)string;
	if (prog->regstart != '\0')
		/* We know what char it must start with. */
		while ((s = strchr(s, prog->regstart)) != NULL) {
			if (regtry(_reglobs, prog, s)) {
				return 1L;
			}
			s++;
		}
	else
		/* We don't -- general case. */
		do {
			if (regtry(_reglobs, prog, s)) {
				return 1L;
			}
		} while (*s++ != '\0');

	/* Failure. */
	return 0L;
}

/*
 - regtry - try match at specific point
 */
static BOOL regtry(re_globals *_reglobs, const regexp *prog, STRPTR string)
{
register LONG i;
register STRPTR *sp;
register STRPTR *ep;

	(_reglobs->reginput) = (STRPTR)string;				/* XXX */
	(_reglobs->regstartp) = (STRPTR*)prog->startp;			/* XXX */
	(_reglobs->regendp) = (STRPTR*)prog->endp;				/* XXX */

	sp = (STRPTR*)prog->startp;				/* XXX */
	ep = (STRPTR*)prog->endp;				/* XXX */
	for (i = NSUBEXP; i > 0; i--) {
		*sp++ = NULL;
		*ep++ = NULL;
	}
	if (regmatch(_reglobs, (STRPTR)prog->program + 1)) {		/* XXX */
		((regexp *)prog)->startp[0] = (STRPTR)string;	/* XXX */
		((regexp *)prog)->endp[0] = (_reglobs->reginput);		/* XXX */
		return TRUE;
	} else
		return FALSE;
}

/*
 - regmatch - main matching routine
 *
 * Conceptually the strategy is simple:  check to see whether the current
 * node matches, call self recursively to see whether the rest matches,
 * and then act accordingly.  In practice we make some effort to avoid
 * recursion, in particular by going through "ordinary" nodes (that don't
 * need to know whether the rest of the match failed) by a loop instead of
 * by recursion.
 */
static BOOL regmatch(re_globals *_reglobs, STRPTR prog)
{
	register UBYTE *scan;	/* Current node. */
	UBYTE *next;		/* Next node. */

	scan = prog;
#ifdef DEBUG
	if (scan != NULL && regnarrate)
		fprintf(stderr, "%s(\n", regprop(scan));
#endif
	while (scan != NULL) {
#ifdef DEBUG
		if (regnarrate)
			fprintf(stderr, "%s...\n", regprop(scan));
#endif
		next = regnext(scan);

		switch (OP(scan)) {
		case BOL:
			if ((_reglobs->reginput) != (_reglobs->regbol))
				return FALSE;
			break;
		case EOL:
			if (*(_reglobs->reginput) != '\0')
				return FALSE;
			break;
		case WORDA:
			/* Must be looking at a letter, digit, or _ */
			if ((!isalnum(*(_reglobs->reginput))) && *(_reglobs->reginput) != '_')
				return FALSE;
			/* Prev must be BOL or nonword */
			if ((_reglobs->reginput) > (_reglobs->regbol) &&
			    (isalnum((_reglobs->reginput)[-1]) || (_reglobs->reginput)[-1] == '_'))
				return FALSE;
			break;
		case WORDZ:
			/* Must be looking at non letter, digit, or _ */
			if (isalnum(*(_reglobs->reginput)) || *(_reglobs->reginput) == '_')
				return FALSE;
			/* We don't care what the previous char was */
			break;
		case ANY:
			if (*(_reglobs->reginput) == '\0')
				return FALSE;
			(_reglobs->reginput)++;
			break;
		case EXACTLY: {
				register LONG len;
				register UBYTE *opnd;

				opnd = OPERAND(scan);
				/* Inline the first character, for speed. */
				if (*opnd != *(_reglobs->reginput))
					return FALSE;
				len = strlen(opnd);
				if (len > 1 && strncmp(opnd, (_reglobs->reginput), len) != 0)
					return FALSE;
				(_reglobs->reginput) += len;
			}
			break;
		case ANYOF:
 			if (*(_reglobs->reginput) == '\0' || strchr(OPERAND(scan), *(_reglobs->reginput)) == NULL)
				return FALSE;
			(_reglobs->reginput)++;
			break;
		case ANYBUT:
 			if (*(_reglobs->reginput) == '\0' || strchr(OPERAND(scan), *(_reglobs->reginput)) != NULL)
				return FALSE;
			(_reglobs->reginput)++;
			break;
		case NOTHING:
			break;
		case BACK:
			break;
		case OPEN+1:
		case OPEN+2:
		case OPEN+3:
		case OPEN+4:
		case OPEN+5:
		case OPEN+6:
		case OPEN+7:
		case OPEN+8:
		case OPEN+9: {
				register LONG no;
				register UBYTE *save;

				no = OP(scan) - OPEN;
				save = (_reglobs->reginput);

				if (regmatch(_reglobs, next)) {
					/*
					 * Don't set startp if some later
					 * invocation of the same parentheses
					 * already has.
					 */
					if ((_reglobs->regstartp)[no] == NULL)
						(_reglobs->regstartp)[no] = save;
					return TRUE;
				} else
					return FALSE;
			}
			break;
		case CLOSE+1:
		case CLOSE+2:
		case CLOSE+3:
		case CLOSE+4:
		case CLOSE+5:
		case CLOSE+6:
		case CLOSE+7:
		case CLOSE+8:
		case CLOSE+9: {
				register LONG no;
				register UBYTE *save;

				no = OP(scan) - CLOSE;
				save = (_reglobs->reginput);

				if (regmatch(_reglobs, next)) {
					/*
					 * Don't set endp if some later
					 * invocation of the same parentheses
					 * already has.
					 */
					if ((_reglobs->regendp)[no] == NULL)
						(_reglobs->regendp)[no] = save;
					return TRUE;
				} else
					return FALSE;
			}
			break;
		case BRANCH: {
				register UBYTE *save;

				if (OP(next) != BRANCH)		/* No choice. */
					next = OPERAND(scan);	/* Avoid recursion. */
				else {
					do {
						save = (_reglobs->reginput);
						if (regmatch(_reglobs, OPERAND(scan)))
							return TRUE;
						(_reglobs->reginput) = save;
						scan = regnext(scan);
					} while (scan != NULL && OP(scan) == BRANCH);
					return FALSE;
					/* NOTREACHED */
				}
			}
			break;
		case STAR:
		case PLUS: {
				register UBYTE nextch;
				register LONG no;
				register UBYTE *save;
				register LONG min;

				/*
				 * Lookahead to avoid useless match attempts
				 * when we know what character comes next.
				 */
				nextch = '\0';
				if (OP(next) == EXACTLY)
					nextch = *OPERAND(next);
				min = (OP(scan) == STAR) ? 0 : 1;
				save = (_reglobs->reginput);
				no = regrepeat(_reglobs, (OPERAND(scan)));
				while (no >= min) {
					/* If it could work, try it. */
					if (nextch == '\0' || *(_reglobs->reginput) == nextch)
						if (regmatch(_reglobs, next))
							return TRUE;
					/* Couldn't or didn't -- back up. */
					no--;
					(_reglobs->reginput) = save + no;
				}
				return FALSE;
			}
			break;
		case END:
			return TRUE;	/* Success! */
			break;
		default:
			FAIL(REXPERR_CORRUPTEDMEM,FALSE);
			break;
		}

		scan = next;
	}

	/*
	 * We get here only if there's trouble -- normally "case END" is
	 * the terminating point.
	 */
	FAIL(REXPERR_CORRUPTEDPTRS,FALSE);
}

/*
 - regrepeat - repeatedly match something simple, report how many
 */
static LONG regrepeat(re_globals *_reglobs, STRPTR p)
{
register LONG count = 0;
register STRPTR scan;
register STRPTR opnd;

	scan = (_reglobs->reginput);
	opnd = OPERAND(p);
	switch (OP(p)) {
	case ANY:
		count = strlen(scan);
		scan += count;
		break;
	case EXACTLY:
		while (*opnd == *scan) {
			count++;
			scan++;
		}
		break;
	case ANYOF:
		while (*scan != '\0' && strchr(opnd, *scan) != NULL) {
			count++;
			scan++;
		}
		break;
	case ANYBUT:
		while (*scan != '\0' && strchr(opnd, *scan) == NULL) {
			count++;
			scan++;
		}
		break;
	default:		/* Oh dear.  Called inappropriately. */
		SetIoErr(REXPERR_INTERNALFOULUP);
		count = 0;	/* Best compromise. */
		break;
	}
	(_reglobs->reginput) = scan;

	return(count);
}

/*
 - regnext - dig the "next" pointer out of a node
 */
static STRPTR regnext(register STRPTR p)
{
register LONG offset;

	if (p == &regdummy)
		return(NULL);

	offset = NEXT(p);
	if (offset == 0)
		return(NULL);

	if (OP(p) == BACK)
		return(p-offset);
	else
		return(p+offset);
}


__saveds __asm STRPTR RegXlatError(register __d0 LONG err)
{
static STRPTR texts[] =
{
	"NULL argument",
	"Regexp too big",
	"Too many parentheses",
	"Unmatched parentheses",
	"Junk on end",
	"Repetition/Kleene (+*) could be empty",
	"Nested postfix (+*?) operators",
	"Invalid range",
	"Unmatched range bracket",
	"Internal urp",
	"Postfix operator (+*?) follows nothing",
	"Trailing backslash",
	"Corrupted program",
	"Corrupted memory",
	"Corrupted pointers",
	"Internal foulup"
};
	return texts[(-err)-1];
}
