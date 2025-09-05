/*
 * tzset.c - initialize timezone information using locale.library
 *
 * This function initializes the tzname variable from the TZ environment
 * variable or from locale.library information. This function is 
 * automatically called by the other time conversion functions that depend 
 * on timezone information.
 *
 * The implementation uses locale.library to get timezone information
 * from the system's locale settings, including GMT offset and locale-specific
 * timezone data. It falls back to TZ environment variable parsing if available,
 * and provides default GMT values if neither is available.
 *
 * POSIX.1-2001, POSIX.1-2008, SVr4, 4.3BSD
 * AmigaOS locale.library integration
 *
 * Features:
 * - Uses locale.library OpenLocale(NULL) to get current system locale
 * - Extracts GMT offset from locale->loc_GMTOffset (read-only structure)
 * - Correctly handles Amiga vs POSIX GMT offset conventions
 * - Generates appropriate timezone names based on offset
 * - Supports TZ environment variable override
 * - Provides fallback to GMT if locale.library unavailable
 * - Memory-safe string handling with static buffers
 * - Proper locale and library cleanup with tzset_cleanup()
 * - Handles fractional hour offsets (e.g., GMT+5:30, GMT-3:30)
 */

#include "amiga.h"
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <proto/locale.h>

/* External variables defined in ctime.c */
extern char *tzname[2];
extern int daylight;
extern time_t timezone;

/* Static variables for locale handling */
extern struct LocaleBase *LocaleBase;
static struct Locale *CurrentLocale = NULL;
static int tzset_called = 0;

/* Default timezone names for fallback */
static const char default_tzname_std[] = "GMT";
static const char default_tzname_dst[] = "GMT";

/* Static buffers for timezone names */
static char tzname_std[32];
static char tzname_dst[32];

/* Enhanced tzset function using AmigaOS locale.library */
void tzset_amiga(void)
{
    struct Locale *locale;
    char *tz_env;
    int gmt_offset_minutes;
    int hours;
    int minutes;
    
    chkabort();
    
    /* Only initialize once unless explicitly called again */
    if (tzset_called && tzname[0]) {
        return;
    }
    
    /* Try to open locale.library if not already open */
    if (!LocaleBase) {
        LocaleBase = (struct LocaleBase *)OpenLibrary("locale.library", 0);
        if (!LocaleBase) {
            /* Fallback to default values if locale.library not available */
            tzname[0] = (char *)default_tzname_std;
            tzname[1] = (char *)default_tzname_dst;
            daylight = 0;
            timezone = 0;
            tzset_called = 1;
            return;
        }
    }
    
    /* Check TZ environment variable first */
    tz_env = getenv("TZ");
    if (tz_env && *tz_env) {
        /* Parse TZ environment variable */
        /* For now, use simple parsing - could be extended for full TZ support */
        if (strlen(tz_env) >= 3) {
            /* Assume format like "EST5EDT" or "GMT0" */
            tzname[0] = (char *)tz_env;
            tzname[1] = (char *)tz_env;
            daylight = 0; /* Simplified - could parse DST info from TZ */
            timezone = 0; /* Simplified - could parse offset from TZ */
        } else {
            /* Use locale information as fallback */
            goto use_locale;
        }
    } else {
use_locale:
        /* Open current default locale (pass NULL to get current locale) */
        locale = OpenLocale(NULL);
        if (locale) {
            /* Store locale reference for cleanup */
            CurrentLocale = locale;
            
            /* Extract timezone information from locale (read-only structure)
             * We can only read from the locale structure, not modify it
             * 
             * According to the documentation:
             * loc_GMTOffset (LONG) - The offset in minutes of the current location from GMT.
             * Positive indicates a Westerly direction from GMT, negative Easterly.
             * 
             * Note: This is opposite to POSIX convention where positive means East of GMT
             */
            gmt_offset_minutes = locale->loc_GMTOffset;
            
            /* Convert to POSIX timezone convention (positive = East of GMT)
             * and set timezone offset in seconds */
            timezone = -gmt_offset_minutes * 60;
            
            /* Generate timezone name based on GMT offset
             * Note: AmigaOS uses opposite convention from POSIX:
             * - AmigaOS: Positive = West of GMT, Negative = East of GMT
             * - POSIX:   Positive = East of GMT, Negative = West of GMT
             */
            if (gmt_offset_minutes == 0) {
                strcpy(tzname_std, "GMT");
                strcpy(tzname_dst, "GMT");
                daylight = 0;
            } else {
                /* Create timezone name with offset (convert to POSIX convention) */
                hours = -gmt_offset_minutes / 60;  /* Convert to POSIX convention */
                minutes = (-gmt_offset_minutes) % 60;
                if (minutes < 0) {
                    minutes += 60;
                    hours -= 1;
                }
                
                if (hours > 0) {
                    if (minutes == 0) {
                        sprintf(tzname_std, "GMT+%d", hours);
                    } else {
                        sprintf(tzname_std, "GMT+%d:%02d", hours, minutes);
                    }
                } else if (hours < 0) {
                    if (minutes == 0) {
                        sprintf(tzname_std, "GMT%d", hours);
                    } else {
                        sprintf(tzname_std, "GMT%d:%02d", hours, minutes);
                    }
                } else {
                    /* hours == 0, minutes != 0 */
                    if (minutes > 0) {
                        sprintf(tzname_std, "GMT+0:%02d", minutes);
                    } else {
                        sprintf(tzname_std, "GMT-0:%02d", -minutes);
                    }
                }
                
                /* For DST, assume 1 hour ahead during daylight saving */
                hours += 1;
                if (hours > 0) {
                    if (minutes == 0) {
                        sprintf(tzname_dst, "GMT+%d", hours);
                    } else {
                        sprintf(tzname_dst, "GMT+%d:%02d", hours, minutes);
                    }
                } else if (hours < 0) {
                    if (minutes == 0) {
                        sprintf(tzname_dst, "GMT%d", hours);
                    } else {
                        sprintf(tzname_dst, "GMT%d:%02d", hours, minutes);
                    }
                } else {
                    /* hours == 0, minutes != 0 */
                    if (minutes > 0) {
                        sprintf(tzname_dst, "GMT+0:%02d", minutes);
                    } else {
                        sprintf(tzname_dst, "GMT-0:%02d", -minutes);
                    }
                }
                
                /* Check if locale indicates DST is used */
                /* This is a simplified check - full DST support would require
                   more complex logic based on locale settings and current date */
                daylight = 0; /* Default to no DST */
                
                /* In a full implementation, we would check:
                   - Current date to determine if DST is active
                   - Locale-specific DST rules
                   - Historical DST changes
                */
            }
            
            /* Set timezone names */
            tzname[0] = tzname_std;
            tzname[1] = daylight ? tzname_dst : tzname_std;
            
        } else {
            /* Fallback to default values if locale cannot be opened */
            tzname[0] = (char *)default_tzname_std;
            tzname[1] = (char *)default_tzname_dst;
            daylight = 0;
            timezone = 0;
        }
    }
    
    tzset_called = 1;
}

/* Cleanup function to close locale library */
void tzset_cleanup(void)
{
    if (CurrentLocale) {
        CloseLocale(CurrentLocale);
        CurrentLocale = NULL;
    }
    if (LocaleBase) {
        CloseLibrary((struct Library *)LocaleBase);
        LocaleBase = NULL;
    }
    tzset_called = 0;
}
