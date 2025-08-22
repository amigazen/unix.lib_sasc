#include <stdio.h>
#include "longlong.h"

/* Helper function to create long_long_t values for C89 compatibility */
long_long_t make_long_long(unsigned long hi, unsigned long lo)
{
    long_long_t result;
    result.hi = hi;
    result.lo = lo;
    return result;
}

/* Test the overflow detection logic */
int main(void)
{
    long_long_t test_values[5];
    int bases[3];
    int digits[4];
    int i, j, k;
    long_long_t current, base_ll, max_before_mul, max_before_add;
    long_long_t current_times_base;
    int base;
    int digit;
    
    /* Initialize test values */
    test_values[0] = make_long_long(0, 0);                    /* 0 */
    test_values[1] = make_long_long(0, 100);                  /* 100 */
    test_values[2] = make_long_long(0, 0x7FFFFFFF);          /* MAX_INT */
    test_values[3] = make_long_long(1, 0);                    /* 2^32 */
    test_values[4] = make_long_long(0x7FFFFFFF, 0xFFFFFFFF);  /* MAX_LONG_LONG */
    
    /* Initialize bases and digits */
    bases[0] = 2;
    bases[1] = 10;
    bases[2] = 16;
    
    digits[0] = 0;
    digits[1] = 1;
    digits[2] = 9;
    digits[3] = 15;
    
    printf("Testing overflow detection logic...\n\n");
    
    for (i = 0; i < 5; i++) {
        for (j = 0; j < 3; j++) {
            for (k = 0; k < 4; k++) {
                current = test_values[i];
                base = bases[j];
                digit = digits[k];
                
                printf("Testing: current=0x%08lX%08lX, base=%d, digit=%d\n", 
                       current.hi, current.lo, base, digit);
                
                /* Check if this combination would overflow */
                base_ll = long_to_long_long(base);
                max_before_mul = long_long_div(get_long_long_max(), base_ll);
                
                if (long_long_lt(current, max_before_mul)) {
                    printf("  -> Safe to multiply (current < max_before_mul)\n");
                } else if (long_long_eq(current, max_before_mul)) {
                    printf("  -> At boundary, need to check digit addition\n");
                    current_times_base = long_long_mul(current, base_ll);
                    max_before_add = long_long_sub(get_long_long_max(), long_to_long_long(digit));
                    if (long_long_gt(long_to_long_long(digit), max_before_add)) {
                        printf("  -> OVERFLOW: digit too large\n");
                    } else {
                        printf("  -> Safe: digit fits\n");
                    }
                } else {
                    printf("  -> OVERFLOW: current > max_before_mul\n");
                }
                printf("\n");
            }
        }
    }
    
    return 0;
}
