/*
 * ffs.c - find first bit set (POSIX compliant)
 *
 * This function returns the position of the first (least significant)
 * bit set in the word i. The least significant bit is position 1
 * and the most significant bit is position 32.
 *
 * POSIX.1-2001, POSIX.1-2008, 4.3BSD
 */

#include "amiga.h"
#include <strings.h>

int ffs(int i)
{
    int bit;
    
    chkabort();
    
    if (i == 0) {
        return 0;
    }
    
    /* Find the first bit set */
    for (bit = 1; bit <= 32; bit++) {
        if (i & (1 << (bit - 1))) {
            return bit;
        }
    }
    
    return 0;
}
