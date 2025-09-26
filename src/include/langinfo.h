/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * langinfo.h - Language information
 * 
 * This header provides language information functions for Amiga
 * 
 * Copyright (C) 2025 by amigazen project
 */

#ifndef _LANGINFO_H
#define _LANGINFO_H 1

#include <nl_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Language information constants */
#define CODESET     0   /* Codeset name */
#define D_T_FMT     1   /* Date and time format */
#define D_FMT       2   /* Date format */
#define T_FMT       3   /* Time format */
#define T_FMT_AMPM  4   /* 12-hour time format */
#define AM_STR      5   /* Ante meridiem string */
#define PM_STR      6   /* Post meridiem string */
#define DAY_1       7   /* Day 1 name */
#define DAY_2       8   /* Day 2 name */
#define DAY_3       9   /* Day 3 name */
#define DAY_4       10  /* Day 4 name */
#define DAY_5       11  /* Day 5 name */
#define DAY_6       12  /* Day 6 name */
#define DAY_7       13  /* Day 7 name */
#define ABDAY_1     14  /* Abbreviated day 1 name */
#define ABDAY_2     15  /* Abbreviated day 2 name */
#define ABDAY_3     16  /* Abbreviated day 3 name */
#define ABDAY_4     17  /* Abbreviated day 4 name */
#define ABDAY_5     18  /* Abbreviated day 5 name */
#define ABDAY_6     19  /* Abbreviated day 6 name */
#define ABDAY_7     20  /* Abbreviated day 7 name */
#define MON_1       21  /* Month 1 name */
#define MON_2       22  /* Month 2 name */
#define MON_3       23  /* Month 3 name */
#define MON_4       24  /* Month 4 name */
#define MON_5       25  /* Month 5 name */
#define MON_6       26  /* Month 6 name */
#define MON_7       27  /* Month 7 name */
#define MON_8       28  /* Month 8 name */
#define MON_9       29  /* Month 9 name */
#define MON_10      30  /* Month 10 name */
#define MON_11      31  /* Month 11 name */
#define MON_12      32  /* Month 12 name */
#define ABMON_1     33  /* Abbreviated month 1 name */
#define ABMON_2     34  /* Abbreviated month 2 name */
#define ABMON_3     35  /* Abbreviated month 3 name */
#define ABMON_4     36  /* Abbreviated month 4 name */
#define ABMON_5     37  /* Abbreviated month 5 name */
#define ABMON_6     38  /* Abbreviated month 6 name */
#define ABMON_7     39  /* Abbreviated month 7 name */
#define ABMON_8     40  /* Abbreviated month 8 name */
#define ABMON_9     41  /* Abbreviated month 9 name */
#define ABMON_10    42  /* Abbreviated month 10 name */
#define ABMON_11    43  /* Abbreviated month 11 name */
#define ABMON_12    44  /* Abbreviated month 12 name */
#define ERA         45  /* Era description */
#define ERA_D_FMT   46  /* Era date format */
#define ERA_D_T_FMT 47  /* Era date and time format */
#define ERA_T_FMT   48  /* Era time format */
#define ALT_DIGITS  49  /* Alternative symbols for digits */
#define RADIXCHAR   50  /* Radix character */
#define THOUSEP     51  /* Separator for thousands */
#define YESEXPR     52  /* Affirmative response expression */
#define NOEXPR      53  /* Negative response expression */
#define CRNCYSTR    54  /* Currency symbol */
#define D_MD_ORDER  55  /* Month/day order */

/* Function prototypes */
char *nl_langinfo(nl_item item);

#ifdef __cplusplus
}
#endif

#endif /* _LANGINFO_H */
