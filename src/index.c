#include <string.h>

/*
 * Modern POSIX-compatible implementation of strchr.
 * The index() function is deprecated in modern POSIX standards.
 * This implementation provides backward compatibility while following
 * modern POSIX conventions.
 */

char *index(const char *str, int c)
{
  /* 
   * Use strchr for the actual implementation since index() is deprecated.
   * This maintains backward compatibility while following modern standards.
   * 
   * Note: index() is equivalent to strchr() but is deprecated in POSIX.1-2001
   * and removed in POSIX.1-2008. Modern code should use strchr() instead.
   */
  return strchr(str, c);
}
