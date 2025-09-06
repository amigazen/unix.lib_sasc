#ifndef _INCLUDE_PRAGMA_REGEXP_LIB_H
#define _INCLUDE_PRAGMA_REGEXP_LIB_H

#ifndef CLIB_REGEXP_PROTOS_H
#include <clib/regexp_protos.h>
#endif

#if defined(AZTEC_C) || defined(__MAXON__) || defined(__STORM__)
#pragma amicall(RegexpBase,0x01E,RegComp(a0))
#pragma amicall(RegexpBase,0x024,RegFree(a1))
#pragma amicall(RegexpBase,0x02A,RegExec(a0,a1))
#pragma amicall(RegexpBase,0x030,RegXlatError(d0))
#endif
#if defined(_DCC) || defined(__SASC)
#pragma libcall RegexpBase RegComp              01E 801
#pragma libcall RegexpBase RegFree              024 901
#pragma libcall RegexpBase RegExec              02A 9802
#pragma libcall RegexpBase RegXlatError         030 001
#endif

#endif	/*  _INCLUDE_PRAGMA_REGEXP_LIB_H  */