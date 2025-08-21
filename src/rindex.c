#include <string.h>

/*
 * Modern POSIX-compatible implementation of strrchr.
 * The rindex() function is deprecated in modern POSIX standards.
 * This implementation provides backward compatibility while following
 * modern POSIX conventions.
 */

char *rindex(const char *str, int c)
{
  /* 
   * Use strrchr for the actual implementation since rindex() is deprecated.
   * This maintains backward compatibility while following modern standards.
   * 
   * Note: rindex() is equivalent to strrchr() but is deprecated in POSIX.1-2001
   * and removed in POSIX.1-2008. Modern code should use strrchr() instead.
   * 
   * The 'r' in rindex/strrchr stands for "rightmost" - it finds the last
   * occurrence of character 'c' in string 'str', searching from right to left.
   */
  return strrchr(str, c);
}
