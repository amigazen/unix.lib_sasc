#include "amiga.h"
#include <string.h>

#undef bzero /* undef SAS/C bzero macro */

/*
 * Zeros out the first n bytes of the memory area pointed to by s.
 */
void bzero(void *s, size_t n)
{
  // A zero-length operation does nothing.
  if (n == 0) {
    return;
  }

  // memset is the standard C function for filling a block of
  // memory with a constant byte.
  memset(s, 0, n);
}