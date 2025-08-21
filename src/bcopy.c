#include "amiga.h"
#include <string.h> 

#undef bcopy /* undef SAS/C bcopy macro */

/*
 * Copies n bytes from src to dest. Handles overlapping memory correctly.
 */
void bcopy(const void *src, void *dest, size_t n)
{
  // A zero-length copy does nothing.
  if (n == 0) {
    return;
  }

  // memmove is the standard C function that does exactly what bcopy
  // is supposed to do, including handling overlapping memory.
  memmove(dest, src, n);
}