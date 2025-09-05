/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * random.c - Random number functions using utility.library FastRand() and RangeRand()
 * 
 * This file implements POSIX random number functions using Amiga's
 * utility.library FastRand() and RangeRand() functions.
 * 
 * Functions implemented:
 * - rand(), srand() - Standard C random number functions
 * - random(), srandom() - POSIX random number functions
 * 
 * C89 compliant for AmigaOS compatibility
 */

#include "amiga.h"
#include <proto/utility.h>
#include <stdlib.h>
#include <time.h>

/* Internal state for random number generation */
static unsigned long random_seed = 1;
static int random_initialized = 0;

/* Forward declarations */
static void _init_random(void);
static unsigned long _amiga_random(void);

/*
 * _init_random - Initialize random number generator
 * 
 * This function initializes the random number generator using
 * the current time as a seed if not already initialized.
 */
static void _init_random(void)
{
    if (!random_initialized) {
        /* Use current time as seed */
        random_seed = (unsigned long)time(NULL);
        random_initialized = 1;
    }
}

/*
 * _amiga_random - Generate random number using utility.library
 * 
 * This function generates a random number using Amiga's utility.library
 * FastRand() function with the current seed.
 * 
 * @return Random number
 */
static unsigned long _amiga_random(void)
{
    unsigned long result;
    
    /* Initialize if needed */
    _init_random();
    
    /* Use utility.library FastRand() with current seed */
    result = FastRand(random_seed);
    
    /* Update seed for next call */
    random_seed = result;
    
    return result;
}

/*
 * _amiga_range_random - Generate random number in specific range using RangeRand()
 * 
 * This function generates a random number in a specific range using Amiga's
 * utility.library RangeRand() function.
 * 
 * @param max_value Maximum value (exclusive)
 * @return Random number in range [0, max_value)
 */
static unsigned long _amiga_range_random(unsigned long max_value)
{
    unsigned long result;
    
    /* Initialize if needed */
    _init_random();
    
    /* RangeRand() only accepts UWORD (0-65535), so we need to handle larger ranges */
    if (max_value <= 65535) {
        /* Use RangeRand() directly for small ranges */
        result = RangeRand((UWORD)max_value);
    } else {
        /* For larger ranges, use FastRand() and scale down */
        result = FastRand(random_seed);
        random_seed = result;
        result = result % max_value;
    }
    
    return result;
}

/*
 * rand - Generate random number (0 to RAND_MAX)
 * 
 * This function generates a random number in the range 0 to RAND_MAX
 * using Amiga's utility.library FastRand() and RangeRand() functions.
 * 
 * @return Random number between 0 and RAND_MAX
 */
int rand(void)
{
    unsigned long result;
    
    /* Check for abort signal */
    __chkabort();
    
    /* Generate random number in RAND_MAX range */
    result = _amiga_range_random(RAND_MAX + 1);
    
    return (int)result;
}

/*
 * srand - Set random number seed
 * 
 * This function sets the seed for the random number generator.
 * 
 * @param seed Seed value
 */
void srand(unsigned int seed)
{
    /* Check for abort signal */
    __chkabort();
    
    /* Set seed and mark as initialized */
    random_seed = (unsigned long)seed;
    random_initialized = 1;
}

/*
 * random - Generate random number (0 to 2^31-1)
 * 
 * This function generates a random number in the range 0 to 2^31-1
 * using Amiga's utility.library FastRand() and RangeRand() functions.
 * 
 * @return Random number between 0 and 2^31-1
 */
long random(void)
{
    unsigned long result;
    
    /* Check for abort signal */
    __chkabort();
    
    /* Generate random number in 2^31-1 range */
    result = _amiga_range_random(2147483647L);
    
    return (long)result;
}

/*
 * srandom - Set random number seed
 * 
 * This function sets the seed for the random number generator.
 * 
 * @param seed Seed value
 */
void srandom(unsigned int seed)
{
    /* Check for abort signal */
    __chkabort();
    
    /* Set seed and mark as initialized */
    random_seed = (unsigned long)seed;
    random_initialized = 1;
}

/*
 * initstate - Initialize random number generator state
 * 
 * This function initializes the random number generator state.
 * For compatibility, we use a simple implementation.
 * 
 * @param seed Seed value
 * @param state State array (unused in this implementation)
 * @param n Size of state array (unused in this implementation)
 * @return Pointer to previous state array
 */
char *initstate(unsigned int seed, char *state, size_t n)
{
    /* Check for abort signal */
    __chkabort();
    
    /* Set seed */
    srandom(seed);
    
    /* Return NULL as we don't maintain state array */
    return NULL;
}

/*
 * setstate - Set random number generator state
 * 
 * This function sets the random number generator state.
 * For compatibility, we use a simple implementation.
 * 
 * @param state State array (unused in this implementation)
 * @return Pointer to previous state array
 */
char *setstate(char *state)
{
    /* Check for abort signal */
    __chkabort();
    
    /* Return NULL as we don't maintain state array */
    return NULL;
}
