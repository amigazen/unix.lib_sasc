/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * rand_r.c - Thread-safe random number generation (POSIX.1-2001)
 * 
 * This function implements a thread-safe version of rand() that uses
 * a user-provided seed pointer instead of global state.
 * 
 * POSIX.1-2001: thread-safe random number generation
 */

#include <stdlib.h>
#include <stddef.h>

#ifndef RAND_MAX
#define RAND_MAX 32767
#endif

/**
 * @brief Thread-safe random number generation
 * @param seedp Pointer to seed value
 * @return Random number between 0 and RAND_MAX
 * 
 * This function generates a random number using a linear congruential
 * generator (LCG) algorithm. The seed is maintained in the location
 * pointed to by seedp, making this function thread-safe.
 * 
 * The LCG formula used is:
 *   next = (a * current + c) mod m
 * where a = 1103515245, c = 12345, m = 2^31
 * 
 * This is the same algorithm used by many standard C libraries.
 */
int rand_r(unsigned int *seedp)
{
    unsigned int seed;
    
    if (seedp == NULL) {
        return 0;  /* Invalid seed pointer */
    }
    
    seed = *seedp;
    
    /* Linear Congruential Generator (LCG) */
    /* Formula: next = (a * current + c) mod m */
    /* where a = 1103515245, c = 12345, m = 2^31 */
    seed = (1103515245U * seed + 12345U) & 0x7FFFFFFF;
    
    /* Update the seed */
    *seedp = seed;
    
    /* Return value in range [0, RAND_MAX] */
    return (int)(seed % (RAND_MAX + 1));
}
