#ifndef  CLIB_REGEXP_PROTOS_H
#define  CLIB_REGEXP_PROTOS_H

/*
**	$VER: regexp_protos.h 38.1 (26.2.98)
**
**	(C) Copyright 1998 Matthias Bethke
*/

#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef LIBRARIES_REGEXP_H
#include <libraries/regexp.h>
#endif

regexp *RegComp(STRPTR);
void RegFree(regexp*);
LONG RegExec(regexp*,STRPTR);
STRPTR RegXlatError(LONG);

#endif /* CLIB_REGEXP_PROTOS_H */
