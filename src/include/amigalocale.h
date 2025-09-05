/*
 * amigalocale.h - Header for locale.library integration helpers
 *
 * This header provides function declarations for integrating AmigaOS
 * locale.library with POSIX time and date functions.
 */

#ifndef AMIGALOCALE_H
#define AMIGALOCALE_H

/* Initialize locale.library and get current locale */
int locale_init(void);

/* Cleanup locale.library resources */
void locale_cleanup(void);

/* Get localized day name */
const char *locale_get_day_name(int wday, int abbrev);

/* Get localized month name */
const char *locale_get_month_name(int mon, int abbrev);

/* Get AM/PM indicator */
const char *locale_get_ampm(int is_pm);

/* Get locale date format string */
const char *locale_get_date_format(void);

/* Get locale time format string */
const char *locale_get_time_format(void);

/* Get locale date/time format string */
const char *locale_get_datetime_format(void);

/* Check if locale is available */
int locale_is_available(void);

#endif /* AMIGALOCALE_H */
