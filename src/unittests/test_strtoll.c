#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "longlong.h"

/* Test function to print long_long_t values */
void print_long_long(const char *label, long_long_t value)
{
    printf("%s: hi=0x%08lX, lo=0x%08lX\n", label, value.hi, value.lo);
}

/* Helper function to create long_long_t values for C89 compatibility */
long_long_t make_long_long(unsigned long hi, unsigned long lo)
{
    long_long_t result;
    result.hi = hi;
    result.lo = lo;
    return result;
}

/* Test function to compare long_long_t values */
int long_long_equal(long_long_t a, long_long_t b)
{
    return (a.hi == b.hi) && (a.lo == b.lo);
}

int main(void)
{
    char *endptr;
    long_long_t result;
    int errors = 0;
    
    printf("Testing strtoll function...\n\n");
    
    /* Test 1: Basic decimal parsing */
    printf("Test 1: Basic decimal parsing\n");
    result = strtoll("12345", &endptr, 10);
    print_long_long("Result", result);
    printf("Expected: hi=0x00000000, lo=0x00003039\n");
    if (!long_long_equal(result, make_long_long(0, 12345))) {
        printf("ERROR: Basic decimal parsing failed!\n");
        errors++;
    }
    printf("\n");
    
    /* Test 2: Large 32-bit number */
    printf("Test 2: Large 32-bit number\n");
    result = strtoll("2147483647", &endptr, 10);
    print_long_long("Result", result);
    printf("Expected: hi=0x00000000, lo=0x7FFFFFFF\n");
    if (!long_long_equal(result, make_long_long(0, 0x7FFFFFFF))) {
        printf("ERROR: Large 32-bit number parsing failed!\n");
        errors++;
    }
    printf("\n");
    
    /* Test 3: Number larger than 32-bit */
    printf("Test 3: Number larger than 32-bit\n");
    result = strtoll("3000000000", &endptr, 10);
    print_long_long("Result", result);
    printf("Expected: hi=0x00000001, lo=0x2D798000\n");
    if (!long_long_equal(result, make_long_long(1, 0x2D798000))) {
        printf("ERROR: 64-bit number parsing failed!\n");
        errors++;
    }
    printf("\n");
    
    /* Test 4: Maximum 64-bit positive number */
    printf("Test 4: Maximum 64-bit positive number\n");
    result = strtoll("9223372036854775807", &endptr, 10);
    print_long_long("Result", result);
    printf("Expected: hi=0x7FFFFFFF, lo=0xFFFFFFFF\n");
    if (!long_long_equal(result, make_long_long(0x7FFFFFFF, 0xFFFFFFFF))) {
        printf("ERROR: Maximum 64-bit number parsing failed!\n");
        errors++;
    }
    printf("\n");
    
    /* Test 5: Negative number */
    printf("Test 5: Negative number\n");
    result = strtoll("-12345", &endptr, 10);
    print_long_long("Result", result);
    printf("Expected: hi=0xFFFFFFFF, lo=0xFFFFCEC7 (two's complement of 12345)\n");
    /* 12345 = 0x3039, so -12345 = 0xFFFFCEC7 in two's complement */
    if (!long_long_equal(result, make_long_long(0xFFFFFFFF, 0xFFFFCEC7))) {
        printf("ERROR: Negative number parsing failed!\n");
        errors++;
    }
    printf("\n");
    
    /* Test 6: Hexadecimal parsing */
    printf("Test 6: Hexadecimal parsing\n");
    result = strtoll("1A2B3C4D", &endptr, 16);
    print_long_long("Result", result);
    printf("Expected: hi=0x00000000, lo=0x1A2B3C4D\n");
    if (!long_long_equal(result, make_long_long(0, 0x1A2B3C4D))) {
        printf("ERROR: Hexadecimal parsing failed!\n");
        errors++;
    }
    printf("\n");
    
    /* Test 7: Auto-base detection */
    printf("Test 7: Auto-base detection (hex)\n");
    result = strtoll("0x12345678", &endptr, 0);
    print_long_long("Result", result);
    printf("Expected: hi=0x00000000, lo=0x12345678\n");
    if (!long_long_equal(result, make_long_long(0, 0x12345678))) {
        printf("ERROR: Auto-base detection failed!\n");
        errors++;
    }
    printf("\n");
    
    /* Test 8: Overflow test */
    printf("Test 8: Overflow test\n");
    errno = 0;
    result = strtoll("9223372036854775808", &endptr, 10);
    print_long_long("Result", result);
    printf("errno: %d (should be ERANGE=%d)\n", errno, ERANGE);
    if (errno != ERANGE) {
        printf("ERROR: Overflow detection failed!\n");
        errors++;
    }
    printf("\n");
    
    /* Test 9: Critical LONG_LONG_MIN test */
    printf("Test 9: Critical LONG_LONG_MIN test\n");
    errno = 0;
    result = strtoll("-9223372036854775808", &endptr, 10);
    print_long_long("Result", result);
    printf("Expected: hi=0x80000000, lo=0x00000000 (LONG_LONG_MIN)\n");
    if (!long_long_equal(result, make_long_long(0x80000000, 0x00000000))) {
        printf("ERROR: LONG_LONG_MIN parsing failed!\n");
        errors++;
    }
    if (errno != 0) {
        printf("ERROR: LONG_LONG_MIN incorrectly set errno to %d\n", errno);
        errors++;
    }
    printf("\n");
    
    /* Test 9b: Positive LONG_LONG_MIN overflow test */
    printf("Test 9b: Positive LONG_LONG_MIN overflow test\n");
    errno = 0;
    result = strtoll("9223372036854775808", &endptr, 10);
    print_long_long("Result", result);
    printf("Expected: hi=0x7FFFFFFF, lo=0xFFFFFFFF (LONG_LONG_MAX due to overflow)\n");
    if (!long_long_equal(result, make_long_long(0x7FFFFFFF, 0xFFFFFFFF))) {
        printf("ERROR: Positive LONG_LONG_MIN overflow handling failed!\n");
        errors++;
    }
    if (errno != ERANGE) {
        printf("ERROR: Positive LONG_LONG_MIN overflow should set errno to ERANGE\n");
        errors++;
    }
    printf("\n");
    
    /* Test 10: Invalid base */
    printf("Test 10: Invalid base\n");
    errno = 0;
    result = strtoll("123", &endptr, 37);
    print_long_long("Result", result);
    printf("errno: %d (should be EINVAL=%d)\n", errno, EINVAL);
    if (errno != EINVAL) {
        printf("ERROR: Invalid base detection failed!\n");
        errors++;
    }
    printf("\n");
    
    /* Summary */
    printf("Test Summary: %d errors found\n", errors);
    printf("Total tests run: 11\n");
    if (errors == 0) {
        printf("SUCCESS: All tests passed!\n");
    } else {
        printf("FAILURE: Some tests failed!\n");
    }
    
    return (errors == 0) ? 0 : 1;
}
