/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by the University of
 *      California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)strftime.c	5.11 (Berkeley) 2/24/91";
#endif				/* LIBC_SCCS and not lint */

#include <sys/types.h>
#include <sys/time.h>
#include <tzfile.h>
#include <string.h>
#include "amigalocale.h"

/* Fallback English arrays for when locale.library is not available */
static char *afmt[] =
{
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat",
};
static char *Afmt[] =
{
    "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday",
    "Saturday",
};
static char *bfmt[] =
{
    "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep",
    "Oct", "Nov", "Dec",
};
static char *Bfmt[] =
{
    "January", "February", "March", "April", "May", "June", "July",
    "August", "September", "October", "November", "December",
};

static size_t gsize;
static char *pt;
static char tbuf[32];  /* Buffer for temporary string formatting */
static int _add(register char *), _conv(int, int, char), _secs(struct tm *);
static int iso8601wknum(const struct tm *), iso8601year(const struct tm *);
static int weeknumber(const struct tm *, int);

static size_t _fmt(register char *, struct tm *);

size_t
strftime(char *s, size_t maxsize, const char *format, const struct tm *t)
{
    pt = s;
    if ((gsize = maxsize) < 1)
	return (0);
    if (_fmt(format, t)) {
	*pt = '\0';
	return (maxsize - gsize);
    }
    return (0);
}

static size_t
 _fmt(register char *format, struct tm *t)
{
    for (; *format; ++format) {
	if (*format == '%')
	    switch (*++format) {
		case '\0':
		    --format;
		    break;
		case 'A':
		    if (t->tm_wday < 0 || t->tm_wday > 6)
			return (0);
		    if (locale_is_available()) {
			if (!_add((char *)locale_get_day_name(t->tm_wday, 0)))
			    return (0);
		    } else {
			if (!_add(Afmt[t->tm_wday]))
			    return (0);
		    }
		    continue;
		case 'a':
		    if (t->tm_wday < 0 || t->tm_wday > 6)
			return (0);
		    if (locale_is_available()) {
			if (!_add((char *)locale_get_day_name(t->tm_wday, 1)))
			    return (0);
		    } else {
			if (!_add(afmt[t->tm_wday]))
			    return (0);
		    }
		    continue;
		case 'B':
		    if (t->tm_mon < 0 || t->tm_mon > 11)
			return (0);
		    if (locale_is_available()) {
			if (!_add((char *)locale_get_month_name(t->tm_mon, 0)))
			    return (0);
		    } else {
			if (!_add(Bfmt[t->tm_mon]))
			    return (0);
		    }
		    continue;
		case 'b':
		case 'h':
		    if (t->tm_mon < 0 || t->tm_mon > 11)
			return (0);
		    if (locale_is_available()) {
			if (!_add((char *)locale_get_month_name(t->tm_mon, 1)))
			    return (0);
		    } else {
			if (!_add(bfmt[t->tm_mon]))
			    return (0);
		    }
		    continue;
		case 'C':
		    if (!_conv((t->tm_year + TM_YEAR_BASE) / 100, 2, '0'))
			return (0);
		    continue;
		case 'c':
		    if (locale_is_available()) {
			/* Use locale-specific date/time format */
			if (!_fmt(locale_get_datetime_format(), t))
			    return (0);
		    } else {
			if (!_fmt("%m/%d/%y %H:%M:%S", t))
			    return (0);
		    }
		    continue;
		case 'D':
		    if (!_fmt("%m/%d/%y", t))
			return (0);
		    continue;
		case 'd':
		    if (!_conv(t->tm_mday, 2, '0'))
			return (0);
		    continue;
		case 'e':
		    if (!_conv(t->tm_mday, 2, ' '))
			return (0);
		    continue;
		case 'H':
		    if (!_conv(t->tm_hour, 2, '0'))
			return (0);
		    continue;
		case 'I':
		    if (!_conv(t->tm_hour % 12 ?
			       t->tm_hour % 12 : 12, 2, '0'))
			return (0);
		    continue;
		case 'j':
		    if (!_conv(t->tm_yday + 1, 3, '0'))
			return (0);
		    continue;
		case 'k':
		    if (!_conv(t->tm_hour, 2, ' '))
			return (0);
		    continue;
		case 'l':
		    if (!_conv(t->tm_hour % 12 ?
			       t->tm_hour % 12 : 12, 2, ' '))
			return (0);
		    continue;
		case 'M':
		    if (!_conv(t->tm_min, 2, '0'))
			return (0);
		    continue;
		case 'm':
		    if (!_conv(t->tm_mon + 1, 2, '0'))
			return (0);
		    continue;
		case 'n':
		    if (!_add("\n"))
			return (0);
		    continue;
		case 'p':
		    if (locale_is_available()) {
			if (!_add((char *)locale_get_ampm(t->tm_hour >= 12)))
			    return (0);
		    } else {
			if (!_add(t->tm_hour >= 12 ? "PM" : "AM"))
			    return (0);
		    }
		    continue;
		case 'R':
		    if (!_fmt("%H:%M", t))
			return (0);
		    continue;
		case 'r':
		    if (!_fmt("%I:%M:%S %p", t))
			return (0);
		    continue;
		case 'S':
		    if (!_conv(t->tm_sec, 2, '0'))
			return (0);
		    continue;
		case 's':
		    if (!_secs(t))
			return (0);
		    continue;
		case 'T':
		    if (!_fmt("%H:%M:%S", t))
			return (0);
		    continue;
		case 'X':
		    if (locale_is_available()) {
			/* Use locale-specific time format */
			if (!_fmt(locale_get_time_format(), t))
			    return (0);
		    } else {
			if (!_fmt("%H:%M:%S", t))
			    return (0);
		    }
		    continue;
		case 't':
		    if (!_add("\t"))
			return (0);
		    continue;
		case 'U':
		    if (!_conv((t->tm_yday + 7 - t->tm_wday) / 7,
			       2, '0'))
			return (0);
		    continue;
		case 'W':
		    if (!_conv((t->tm_yday + 7 -
				(t->tm_wday ? (t->tm_wday - 1) : 6))
			       / 7, 2, '0'))
			return (0);
		    continue;
		case 'w':
		    if (!_conv(t->tm_wday, 1, '0'))
			return (0);
		    continue;
		case 'x':
		    if (locale_is_available()) {
			/* Use locale-specific date format */
			if (!_fmt(locale_get_date_format(), t))
			    return (0);
		    } else {
			if (!_fmt("%m/%d/%y", t))
			    return (0);
		    }
		    continue;
		case 'y':
		    if (!_conv((t->tm_year + TM_YEAR_BASE)
			       % 100, 2, '0'))
			return (0);
		    continue;
		case 'Y':
		    if (!_conv(t->tm_year + TM_YEAR_BASE, 4, '0'))
			return (0);
		    continue;
		case 'Z':
		    if (!t->tm_zone || !_add(t->tm_zone))
			return (0);
		    continue;
		case 'z':	/* time zone offset east of GMT e.g. -0600 */
		    {
			long off;
			extern time_t timezone;
			extern int daylight;
			
			/* Calculate offset in minutes east of GMT */
			off = -timezone / 60;
			
			if (off < 0) {
			    sprintf(tbuf, "-%02d%02d", (-off) / 60, (-off) % 60);
			} else {
			    sprintf(tbuf, "+%02d%02d", off / 60, off % 60);
			}
			if (!_add(tbuf))
			    return (0);
		    }
		    continue;
		case 'V':	/* week of year according ISO 8601 */
		    {
			int week = iso8601wknum(t);
			if (!_conv(week, 2, '0'))
			    return (0);
		    }
		    continue;
		case 'u':	/* ISO 8601: Weekday as a decimal number [1 (Monday) - 7] */
		    if (!_conv(t->tm_wday == 0 ? 7 : t->tm_wday, 1, '0'))
			return (0);
		    continue;
		case 'G':	/* Year of ISO week */
		case 'g':	/* Year of ISO week (2 digits) */
		    {
			int iso_year = iso8601year(t);
			if (*format == 'g') {
			    if (!_conv(iso_year % 100, 2, '0'))
				return (0);
			} else {
			    if (!_conv(iso_year, 4, '0'))
				return (0);
			}
		    }
		    continue;
		case 'E':	/* POSIX locale extensions, ignored for now */
		case 'O':	/* POSIX locale extensions, ignored for now */
		    /* Skip these for now - could be implemented with locale support */
		    continue;
		case '%':
		    /*
		     * X311J/88-090 (4.12.3.5): if conversion char is
		     * undefined, behavior is undefined.  Print out the
		     * character itself as printf(3) does.
		     */
		default:
		    break;
	    }
	if (!gsize--)
	    return (0);
	*pt++ = *format;
    }
    return (gsize);
}

static _secs(struct tm *t)
{
    static char buf[15];
    register time_t s;
    register char *p;
    struct tm tmp;

    /* Make a copy, mktime(3) modifies the tm struct. */
    tmp = *t;
    s = mktime(&tmp);
    for (p = buf + sizeof(buf) - 2; s > 0 && p > buf; s /= 10)
	*p-- = s % 10 + '0';
    return (_add(++p));
}

static _conv(int n, int digits, char pad)
{
    static char buf[10];
    register char *p;

    for (p = buf + sizeof(buf) - 2; n > 0 && p > buf; n /= 10, --digits)
	*p-- = n % 10 + '0';
    while (p > buf && digits-- > 0)
	*p-- = pad;
    return (_add(++p));
}

static _add(register char *str)
{
    for (;; ++pt, --gsize) {
	if (!gsize)
	    return (0);
	if (!(*pt = *str++))
	    return (1);
    }
}

/* ISO 8601 week number calculation */
static int
iso8601wknum(const struct tm *timeptr)
{
    int weeknum, jan1day, diff;
    int year = timeptr->tm_year + 1900;
    
    /* Get week number, Monday as first day of the week */
    weeknum = weeknumber(timeptr, 1);
    
    /* What day of the week does January 1 fall on? */
    jan1day = timeptr->tm_wday - (timeptr->tm_yday % 7);
    if (jan1day < 0)
	jan1day += 7;
    
    /* If Jan 1 was a Monday through Thursday, it was in week 1 */
    switch (jan1day) {
    case 1: /* Monday */
	break;
    case 2: /* Tuesday */
    case 3: /* Wednesday */
    case 4: /* Thursday */
	weeknum++;
	break;
    case 5: /* Friday */
    case 6: /* Saturday */
    case 0: /* Sunday */
	if (weeknum == 0) {
	    /* This is week 52 or 53 of the previous year */
	    struct tm dec31ly;
	    dec31ly = *timeptr;
	    dec31ly.tm_mday = 31;
	    dec31ly.tm_mon = 11;
	    dec31ly.tm_year = year - 1901;
	    dec31ly.tm_hour = 12;
	    dec31ly.tm_min = 0;
	    dec31ly.tm_sec = 0;
	    dec31ly.tm_isdst = -1;
	    
	    mktime(&dec31ly);
	    weeknum = weeknumber(&dec31ly, 1);
	}
	break;
    }
    
    return weeknum;
}

/* ISO 8601 year calculation */
static int
iso8601year(const struct tm *timeptr)
{
    int weeknum = iso8601wknum(timeptr);
    int year = timeptr->tm_year + 1900;
    
    /* If it's December but the ISO week number is one,
     * that week is in next year */
    if (timeptr->tm_mon == 11 && weeknum == 1) {
	return year + 1;
    }
    
    /* If it's January but the ISO week number is 52 or 53,
     * that week is in last year */
    if (timeptr->tm_mon == 0 && (weeknum == 52 || weeknum == 53)) {
	return year - 1;
    }
    
    return year;
}

/* Week number calculation with configurable first day of week */
static int
weeknumber(const struct tm *timeptr, int firstweekday)
{
    int wday = timeptr->tm_wday;
    int ret;
    
    if (firstweekday == 1) {
	/* Monday is first day of week */
	if (wday == 0)
	    wday = 7;
	wday--;
    }
    
    ret = (timeptr->tm_yday + 7 - wday) / 7;
    return ret;
}
