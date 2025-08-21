#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <limits.h>

/*
 * POSIX-compatible strtol() function implementation for AmigaOS.
 * This function converts a string to a long integer with base conversion.
 * 
 * Note: This is a basic implementation that may need enhancement
 * for full POSIX compliance on AmigaOS.
 */

long strtol(const char *str, char **endptr, int base)
{
  const char *s = str;
  unsigned long acc;
  int c;
  unsigned long cutoff;
  int neg = 0, any, cutlim;

  /* Skip white space and pick up leading +/- sign if any */
  do {
    c = *s++;
  } while (isspace(c));
  
  if (c == '-') {
    neg = 1;
    c = *s++;
  } else if (c == '+') {
    c = *s++;
  }

  /* If base is 0, pick base 8, 10 or 16 */
  if (base == 0) {
    if (c == '0') {
      c = *s++;
      if (c == 'x' || c == 'X') {
        base = 16;
        c = *s++;
      } else {
        base = 8;
      }
    } else {
      base = 10;
    }
  }

  /* Validate base */
  if (base < 2 || base > 36) {
    errno = EINVAL;
    if (endptr) *endptr = (char *)str;
    return 0;
  }

  /* Compute cutoff: largest number that can be represented */
  cutoff = neg ? -(unsigned long)LONG_MIN : LONG_MAX;
  cutlim = cutoff % (unsigned long)base;
  cutoff /= (unsigned long)base;

  for (acc = 0, any = 0;; c = *s++) {
    if (isdigit(c)) {
      c -= '0';
    } else if (isalpha(c)) {
      c -= isupper(c) ? 'A' - 10 : 'a' - 10;
    } else {
      break;
    }
    
    if (c >= base) {
      break;
    }
    
    if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim)) {
      any = -1;
    } else {
      any = 1;
      acc *= base;
      acc += c;
    }
  }

  if (any < 0) {
    acc = neg ? LONG_MIN : LONG_MAX;
    errno = ERANGE;
  } else if (!any) {
    /* No conversion performed */
    if (endptr) *endptr = (char *)str;
    return 0;
  } else if (neg) {
    acc = -acc;
  }

  if (endptr) *endptr = (char *)(any ? s - 1 : str);
  return (long)acc;
}
