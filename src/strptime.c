/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * strptime.c - parse time string (POSIX compliant)
 *
 * The strptime() function parses the string s according to the format
 * string format and fills the tm structure pointed to by tm.
 *
 * POSIX.1-2001, POSIX.1-2008 (XSI extension)
 */

#include "amiga.h"
#include "amigalocale.h"
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

/* Forward declarations */
static int _parse_number(const char **s, int *value, int min, int max);
static int _parse_month(const char **s, int *month);
static int _parse_weekday(const char **s, int *wday);
static int _parse_ampm(const char **s, int *hour);
static int _parse_timezone(const char **s, long *gmtoff);

/* Static arrays for English month and day names (C89 compliance) */
static const char *english_months[12] = {
    "January", "February", "March", "April", "May", "June",
    "July", "August", "September", "October", "November", "December"
};

static const char *english_abbrev_months[12] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

static const char *english_days[7] = {
    "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
};

static const char *english_abbrev_days[7] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

/*
 * strptime() - parse time string
 *
 * The strptime() function parses the string s according to the format
 * string format and fills the tm structure pointed to by tm.
 *
 * Parameters:
 *   s: string to parse
 *   format: format string
 *   tm: pointer to tm structure to fill
 *
 * Returns: pointer to first unparsed character, or NULL on error
 *
 * Note: This implementation supports basic POSIX format specifiers
 * and integrates with Amiga's locale.library for localized parsing.
 */
char *strptime(const char *s, const char *format, struct tm *tm)
{
    const char *s_ptr = s;
    const char *fmt_ptr = format;
    int value;
    
    /* Check for abort signal */
    __chkabort();
    
    /* Validate parameters */
    if (s == NULL || format == NULL || tm == NULL) {
        errno = EINVAL;
        return NULL;
    }
    
    /* Initialize tm structure */
    memset(tm, 0, sizeof(struct tm));
    tm->tm_isdst = -1;  /* Unknown DST status */
    
    /* Parse format string */
    while (*fmt_ptr && *s_ptr) {
        if (*fmt_ptr == '%') {
            fmt_ptr++;  /* Skip '%' */
            
            switch (*fmt_ptr) {
                case 'Y':  /* Full year (4 digits) */
                    if (!_parse_number(&s_ptr, &value, 1900, 9999)) {
                        return NULL;
                    }
                    tm->tm_year = value - 1900;
                    break;
                    
                case 'y':  /* Year without century (2 digits) */
                    if (!_parse_number(&s_ptr, &value, 0, 99)) {
                        return NULL;
                    }
                    tm->tm_year = value < 50 ? value + 100 : value;
                    break;
                    
                case 'm':  /* Month (01-12) */
                    if (!_parse_number(&s_ptr, &value, 1, 12)) {
                        return NULL;
                    }
                    tm->tm_mon = value - 1;
                    break;
                    
                case 'B':  /* Full month name */
                    if (!_parse_month(&s_ptr, &tm->tm_mon)) {
                        return NULL;
                    }
                    break;
                    
                case 'b':  /* Abbreviated month name */
                    if (!_parse_month(&s_ptr, &tm->tm_mon)) {
                        return NULL;
                    }
                    break;
                    
                case 'd':  /* Day of month (01-31) */
                    if (!_parse_number(&s_ptr, &value, 1, 31)) {
                        return NULL;
                    }
                    tm->tm_mday = value;
                    break;
                    
                case 'H':  /* Hour (00-23) */
                    if (!_parse_number(&s_ptr, &value, 0, 23)) {
                        return NULL;
                    }
                    tm->tm_hour = value;
                    break;
                    
                case 'I':  /* Hour (01-12) */
                    if (!_parse_number(&s_ptr, &value, 1, 12)) {
                        return NULL;
                    }
                    tm->tm_hour = value;
                    break;
                    
                case 'M':  /* Minute (00-59) */
                    if (!_parse_number(&s_ptr, &value, 0, 59)) {
                        return NULL;
                    }
                    tm->tm_min = value;
                    break;
                    
                case 'S':  /* Second (00-61) */
                    if (!_parse_number(&s_ptr, &value, 0, 61)) {
                        return NULL;
                    }
                    tm->tm_sec = value;
                    break;
                    
                case 'A':  /* Full weekday name */
                    if (!_parse_weekday(&s_ptr, &tm->tm_wday)) {
                        return NULL;
                    }
                    break;
                    
                case 'a':  /* Abbreviated weekday name */
                    if (!_parse_weekday(&s_ptr, &tm->tm_wday)) {
                        return NULL;
                    }
                    break;
                    
                case 'p':  /* AM/PM indicator */
                    if (!_parse_ampm(&s_ptr, &tm->tm_hour)) {
                        return NULL;
                    }
                    break;
                    
                case 'Z':  /* Timezone name */
                    if (!_parse_timezone(&s_ptr, &tm->tm_gmtoff)) {
                        return NULL;
                    }
                    break;
                    
                case 'z':  /* Timezone offset */
                    if (!_parse_timezone(&s_ptr, &tm->tm_gmtoff)) {
                        return NULL;
                    }
                    break;
                    
                case 'j':  /* Day of year (001-366) */
                    if (!_parse_number(&s_ptr, &value, 1, 366)) {
                        return NULL;
                    }
                    tm->tm_yday = value - 1;
                    break;
                    
                case 'w':  /* Day of week (0-6, Sunday=0) */
                    if (!_parse_number(&s_ptr, &value, 0, 6)) {
                        return NULL;
                    }
                    tm->tm_wday = value;
                    break;
                    
                case 'U':  /* Week number (00-53, Sunday first) */
                case 'W':  /* Week number (00-53, Monday first) */
                    if (!_parse_number(&s_ptr, &value, 0, 53)) {
                        return NULL;
                    }
                    /* Week number is not stored in tm structure */
                    break;
                    
                case 'c':  /* Date and time */
                    /* This is a complex format, for now just skip */
                    while (*s_ptr && !isspace(*s_ptr)) {
                        s_ptr++;
                    }
                    break;
                    
                case 'x':  /* Date */
                    /* This is a complex format, for now just skip */
                    while (*s_ptr && !isspace(*s_ptr)) {
                        s_ptr++;
                    }
                    break;
                    
                case 'X':  /* Time */
                    /* This is a complex format, for now just skip */
                    while (*s_ptr && !isspace(*s_ptr)) {
                        s_ptr++;
                    }
                    break;
                    
                case '%%':  /* Literal '%' */
                    if (*s_ptr != '%') {
                        return NULL;
                    }
                    s_ptr++;
                    break;
                    
                default:
                    /* Unknown format specifier */
                    errno = EINVAL;
                    return NULL;
            }
            fmt_ptr++;
        } else {
            /* Literal character */
            if (*s_ptr != *fmt_ptr) {
                return NULL;
            }
            s_ptr++;
            fmt_ptr++;
        }
    }
    
    /* Check if we consumed the entire format string */
    if (*fmt_ptr) {
        return NULL;
    }
    
    return (char *)s_ptr;
}

/* Parse a number from string */
static int _parse_number(const char **s, int *value, int min, int max)
{
    const char *ptr = *s;
    int result = 0;
    int digits = 0;
    
    /* Skip whitespace */
    while (isspace(*ptr)) {
        ptr++;
    }
    
    /* Parse digits */
    while (isdigit(*ptr)) {
        result = result * 10 + (*ptr - '0');
        ptr++;
        digits++;
    }
    
    /* Check if we got at least one digit */
    if (digits == 0) {
        return 0;
    }
    
    /* Check range */
    if (result < min || result > max) {
        return 0;
    }
    
    *value = result;
    *s = ptr;
    return 1;
}

/* Parse month name */
static int _parse_month(const char **s, int *month)
{
    const char *ptr = *s;
    int i;
    const char *month_name;
    
    /* Skip whitespace */
    while (isspace(*ptr)) {
        ptr++;
    }
    
    /* Try to match month names */
    for (i = 0; i < 12; i++) {
        /* Try full name first */
        if (locale_is_available()) {
            month_name = locale_get_month_name(i, 0);
            if (month_name && strncasecmp(ptr, month_name, strlen(month_name)) == 0) {
                *month = i;
                *s = ptr + strlen(month_name);
                return 1;
            }
            
            /* Try abbreviated name */
            month_name = locale_get_month_name(i, 1);
            if (month_name && strncasecmp(ptr, month_name, strlen(month_name)) == 0) {
                *month = i;
                *s = ptr + strlen(month_name);
                return 1;
            }
        }
        
        /* Fallback to English names */
        if (strncasecmp(ptr, english_months[i], strlen(english_months[i])) == 0) {
            *month = i;
            *s = ptr + strlen(english_months[i]);
            return 1;
        }
        if (strncasecmp(ptr, english_abbrev_months[i], strlen(english_abbrev_months[i])) == 0) {
            *month = i;
            *s = ptr + strlen(english_abbrev_months[i]);
            return 1;
        }
    }
    
    return 0;
}

/* Parse weekday name */
static int _parse_weekday(const char **s, int *wday)
{
    const char *ptr = *s;
    int i;
    const char *day_name;
    
    /* Skip whitespace */
    while (isspace(*ptr)) {
        ptr++;
    }
    
    /* Try to match weekday names */
    for (i = 0; i < 7; i++) {
        /* Try full name first */
        if (locale_is_available()) {
            day_name = locale_get_day_name(i, 0);
            if (day_name && strncasecmp(ptr, day_name, strlen(day_name)) == 0) {
                *wday = i;
                *s = ptr + strlen(day_name);
                return 1;
            }
            
            /* Try abbreviated name */
            day_name = locale_get_day_name(i, 1);
            if (day_name && strncasecmp(ptr, day_name, strlen(day_name)) == 0) {
                *wday = i;
                *s = ptr + strlen(day_name);
                return 1;
            }
        }
        
        /* Fallback to English names */
        if (strncasecmp(ptr, english_days[i], strlen(english_days[i])) == 0) {
            *wday = i;
            *s = ptr + strlen(english_days[i]);
            return 1;
        }
        if (strncasecmp(ptr, english_abbrev_days[i], strlen(english_abbrev_days[i])) == 0) {
            *wday = i;
            *s = ptr + strlen(english_abbrev_days[i]);
            return 1;
        }
    }
    
    return 0;
}

/* Parse AM/PM indicator */
static int _parse_ampm(const char **s, int *hour)
{
    const char *ptr = *s;
    const char *ampm_str;
    
    /* Skip whitespace */
    while (isspace(*ptr)) {
        ptr++;
    }
    
    /* Try to get localized AM/PM string */
    if (locale_is_available()) {
        ampm_str = locale_get_ampm(0);  /* AM */
        if (ampm_str && strncasecmp(ptr, ampm_str, strlen(ampm_str)) == 0) {
            if (*hour == 12) {
                *hour = 0;
            }
            *s = ptr + strlen(ampm_str);
            return 1;
        }
        
        ampm_str = locale_get_ampm(1);  /* PM */
        if (ampm_str && strncasecmp(ptr, ampm_str, strlen(ampm_str)) == 0) {
            if (*hour != 12) {
                *hour += 12;
            }
            *s = ptr + strlen(ampm_str);
            return 1;
        }
    }
    
    /* Fallback to English AM/PM */
    if (strncasecmp(ptr, "AM", 2) == 0) {
        if (*hour == 12) {
            *hour = 0;
        }
        *s = ptr + 2;
        return 1;
    }
    if (strncasecmp(ptr, "PM", 2) == 0) {
        if (*hour != 12) {
            *hour += 12;
        }
        *s = ptr + 2;
        return 1;
    }
    
    return 0;
}

/* Parse timezone */
static int _parse_timezone(const char **s, long *gmtoff)
{
    const char *ptr = *s;
    int sign = 1;
    long hours = 0;
    long minutes = 0;
    
    /* Skip whitespace */
    while (isspace(*ptr)) {
        ptr++;
    }
    
    /* Handle timezone offset format: +HHMM or -HHMM */
    if (*ptr == '+' || *ptr == '-') {
        sign = (*ptr == '+') ? 1 : -1;
        ptr++;
        
        /* Parse hours */
        if (!_parse_number(&ptr, (int*)&hours, 0, 23)) {
            return 0;
        }
        
        /* Parse minutes */
        if (!_parse_number(&ptr, (int*)&minutes, 0, 59)) {
            return 0;
        }
        
        *gmtoff = sign * (hours * 3600 + minutes * 60);
        *s = ptr;
        return 1;
    }
    
    /* For now, just skip timezone names */
    while (*ptr && !isspace(*ptr) && *ptr != '+' && *ptr != '-') {
        ptr++;
    }
    
    *s = ptr;
    return 1;
}
