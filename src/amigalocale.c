/*
 * amigalocale.c - Helper functions for locale.library integration
 *
 * This module provides helper functions for integrating AmigaOS locale.library
 * with POSIX time and date functions. It handles locale initialization,
 * cleanup, and provides localized string retrieval functions.
 *
 * C89 compliant for AmigaOS compatibility
 */

#include "amiga.h"
#include <time.h>
#include <string.h>
#include <proto/locale.h>

/* Static variables for locale handling */
extern struct LocaleBase *LocaleBase = NULL;
static struct Locale *CurrentLocale = NULL;
static int locale_initialized = 0;

/* Initialize locale.library and get current locale */
int locale_init(void)
{
    /* Only initialize once */
    if (locale_initialized) {
        return 1;
    }
    
    /* Try to open locale.library if not already open */
    if (!LocaleBase) {
        LocaleBase = (struct LocaleBase *)OpenLibrary("locale.library", 0);
        if (!LocaleBase) {
            return 0; /* Failed to open locale.library */
        }
    }
    
    /* Open current default locale (pass NULL to get current locale) */
    CurrentLocale = OpenLocale(NULL);
    if (!CurrentLocale) {
        CloseLibrary((struct Library *)LocaleBase);
        LocaleBase = NULL;
        return 0; /* Failed to open locale */
    }
    
    locale_initialized = 1;
    return 1;
}

/* Cleanup locale.library resources */
void locale_cleanup(void)
{
    if (CurrentLocale) {
        CloseLocale(CurrentLocale);
        CurrentLocale = NULL;
    }
    if (LocaleBase) {
        CloseLibrary(LocaleBase);
        LocaleBase = NULL;
    }
    locale_initialized = 0;
}

/* Get localized day name */
const char *locale_get_day_name(int wday, int abbrev)
{
    int locale_id;
    
    /* Validate day of week (0 = Sunday, 6 = Saturday) */
    if (wday < 0 || wday > 6) {
        return abbrev ? "???" : "Unknown";
    }
    
    /* Initialize locale if not already done */
    if (!locale_initialized) {
        if (!locale_init()) {
            return abbrev ? "???" : "Unknown";
        }
    }
    
    /* Get appropriate locale string ID */
    if (abbrev) {
        locale_id = ABDAY_1 + wday;
    } else {
        locale_id = DAY_1 + wday;
    }
    
    /* Get localized string from locale */
    return (const char *)GetLocaleStr(CurrentLocale, locale_id);
}

/* Get localized month name */
const char *locale_get_month_name(int mon, int abbrev)
{
    int locale_id;
    
    /* Validate month (0 = January, 11 = December) */
    if (mon < 0 || mon > 11) {
        return abbrev ? "???" : "Unknown";
    }
    
    /* Initialize locale if not already done */
    if (!locale_initialized) {
        if (!locale_init()) {
            return abbrev ? "???" : "Unknown";
        }
    }
    
    /* Get appropriate locale string ID */
    if (abbrev) {
        locale_id = ABMON_1 + mon;
    } else {
        locale_id = MON_1 + mon;
    }
    
    /* Get localized string from locale */
    return (const char *)GetLocaleStr(CurrentLocale, locale_id);
}

/* Get AM/PM indicator */
const char *locale_get_ampm(int is_pm)
{
    /* Initialize locale if not already done */
    if (!locale_initialized) {
        if (!locale_init()) {
            return is_pm ? "PM" : "AM";
        }
    }
    
    /* Get AM/PM string from locale */
    if (is_pm) {
        return (const char *)GetLocaleStr(CurrentLocale, PM_STR);
    } else {
        return (const char *)GetLocaleStr(CurrentLocale, AM_STR);
    }
}

/* Get locale date format string */
const char *locale_get_date_format(void)
{
    /* Initialize locale if not already done */
    if (!locale_initialized) {
        if (!locale_init()) {
            return "%m/%d/%y"; /* Default US format */
        }
    }
    
    /* Return locale-specific date format */
    return (const char *)CurrentLocale->loc_DateFormat;
}

/* Get locale time format string */
const char *locale_get_time_format(void)
{
    /* Initialize locale if not already done */
    if (!locale_initialized) {
        if (!locale_init()) {
            return "%H:%M:%S"; /* Default 24-hour format */
        }
    }
    
    /* Return locale-specific time format */
    return (const char *)CurrentLocale->loc_TimeFormat;
}

/* Get locale date/time format string */
const char *locale_get_datetime_format(void)
{
    /* Initialize locale if not already done */
    if (!locale_initialized) {
        if (!locale_init()) {
            return "%a %b %e %H:%M:%S %Y"; /* Default format */
        }
    }
    
    /* Return locale-specific date/time format */
    return (const char *)CurrentLocale->loc_DateTimeFormat;
}

/* Check if locale is available */
int locale_is_available(void)
{
    return locale_initialized && (CurrentLocale != NULL);
}
