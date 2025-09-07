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
#include "sc:include/stddef.h"
#else
#error Wrong compiler (SAS/C required)
#endif

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
