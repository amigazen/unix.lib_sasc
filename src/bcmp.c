#include "amiga.h"
#include <string.h>

#undef bcmp /* undef SAS/C bcmp macro */

/*
 * Compares the first n bytes of s1 and s2.
 * Returns 0 if they are identical, non-zero otherwise.
 */
int bcmp(const void *s1, const void *s2, size_t n)
{
  // A zero-length comparison is always true (they are equal).
  if (n == 0) {
    return 0;
  }

  // Use memcmp to do the actual work. Its return value (0 for equal,
  // non-zero for different) is exactly what bcmp requires.
  return memcmp(s1, s2, n);
}