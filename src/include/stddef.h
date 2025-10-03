/*
 * SPDX-License-Identifier: BSD-2-Clause
 * stddef.h - standard definitions
 *
 * This header provides standard definitions including NULL, size_t,
 * and other common types and macros. It includes the SAS/C stddef.h
 * and adds missing POSIX extensions.
 *
 * Copyright (C) 2025 by amigazen project
 */

#ifndef _UNIX_STDDEF_H
#define _UNIX_STDDEF_H

#ifdef __SASC
/* Include SAS/C's built-in stddef.h */
#include "sc:include/stddef.h"
#else

#ifndef STDDEF_H
#define STDDEF_H

#ifndef NULL
#define NULL ((void *)0)
#endif

#ifndef offsetof
#define offsetof(type, member) ((size_t)&((type *)0)->member)
#endif

typedef int ptrdiff_t;
typedef unsigned int size_t;

/* C99 wide character type - properly defined for Amiga */
#ifndef __cplusplus
#ifndef _WCHAR_T_DEFINED
#define _WCHAR_T_DEFINED
typedef unsigned short wchar_t;  /* 16-bit wide character for Amiga */
#endif
#endif

/* C99 maximum alignment type */
#ifndef _MAX_ALIGN_T_DEFINED
#define _MAX_ALIGN_T_DEFINED
typedef struct {
    long long __max_align_ll;
    long double __max_align_ld;
} max_align_t;
#endif

#endif /* STDDEF_H */

#endif /* __SASC */

/* Additional POSIX extensions not provided by SAS/C stddef.h */

/* wchar_t type - wide character type (POSIX) */
#ifndef _WCHAR_T_DEFINED
#define _WCHAR_T_DEFINED
/* wchar_t is defined in SAS/C commchar.h*/
#endif

/* Additional size_t definitions for compatibility */
#ifndef _SIZE_T_DEFINED
#define _SIZE_T_DEFINED
/* size_t should already be defined by SAS/C, but ensure it's available */
#endif

/* Additional NULL definitions for compatibility */
#ifndef _NULL_DEFINED
#define _NULL_DEFINED
/* NULL should already be defined by SAS/C, but ensure it's available */
#endif

/* Additional ptrdiff_t definitions for compatibility */
#ifndef _PTRDIFF_T_DEFINED
#define _PTRDIFF_T_DEFINED
/* ptrdiff_t should already be defined by SAS/C, but ensure it's available */
#endif

/* Additional offsetof definitions for compatibility */
#ifndef _OFFSETOF_DEFINED
#define _OFFSETOF_DEFINED
/* offsetof should already be defined by SAS/C, but ensure it's available */
#endif

#endif /* _UNIX_STDDEF_H */
