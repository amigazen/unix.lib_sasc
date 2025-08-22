#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include "longlong.h"

/*
 * POSIX-compatible strtoll() function implementation for Amiga.
 * This function converts a string to a long long integer with base conversion.
 * 
 * Note: On Amiga with SAS/C, we emulate long long using a struct
 * of two longs to provide true 64-bit integer support.
 */

/* Constants for 64-bit limits */
static const long_long_t LONG_LONG_MAX_VALUE = {0x7FFFFFFF, 0xFFFFFFFF};
static const long_long_t LONG_LONG_MIN_VALUE = {0x80000000, 0x00000000};

/* Helper function to convert a character to its digit value */
static int char_to_digit(char c, int base)
{
    int digit;
    
    if (c >= '0' && c <= '9') {
        digit = c - '0';
    } else if (c >= 'a' && c <= 'z') {
        digit = c - 'a' + 10;
    } else if (c >= 'A' && c <= 'Z') {
        digit = c - 'A' + 10;
    } else {
        return -1; /* Invalid character */
    }
    
    return (digit < base) ? digit : -1;
}

/* Helper function to check for overflow before multiplication and addition */
static int will_overflow(long_long_t current, int base, int digit)
{
    long_long_t base_ll, max_before_mul, max_before_add;
    
    /* Convert base to long_long_t */
    base_ll = long_to_long_long(base);
    
    /* Check if current value is already beyond minimum (most negative) */
    if (long_long_lt(current, LONG_LONG_MIN_VALUE)) {
        return 1;
    }
    
    /* Calculate maximum value before multiplication (working with negative numbers) */
    /* We're working with negative numbers, so we check against LONG_LONG_MIN_VALUE */
    max_before_mul = long_long_div(LONG_LONG_MIN_VALUE, base_ll);
    
    if (long_long_gt(current, max_before_mul)) {
        return 0; /* Safe to multiply */
    }
    
    if (long_long_eq(current, max_before_mul)) {
        /* Check if subtracting digit after multiplication will overflow */
        /* We need to check if: (current * base) - digit < LONG_LONG_MIN */
        /* This is equivalent to: digit > (current * base) - LONG_LONG_MIN */
        /* Since current == max_before_mul, we have: current * base >= LONG_LONG_MIN */
        /* So we need to check if: digit > (current * base) - LONG_LONG_MIN */
        long_long_t current_times_base = long_long_mul(current, base_ll);
        max_before_add = long_long_sub(current_times_base, LONG_LONG_MIN_VALUE);
        return long_long_gt(long_to_long_long(digit), max_before_add);
    }
    
    return 1; /* Will overflow */
}

long_long_t strtoll(const char *str, char **endptr, int base)
{
    long_long_t result;
    int negative = 0;
    int digit;
    const char *start = str;
    
    /* Initialize result to zero */
    result = long_to_long_long(0);
    
    /* Skip leading whitespace */
    while (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r' || *str == '\f' || *str == '\v') {
        str++;
    }
    
    /* Handle sign */
    if (*str == '-') {
        negative = 1;
        str++;
    } else if (*str == '+') {
        str++;
    }
    
    /* Auto-detect base if base is 0 */
    if (base == 0) {
        if (*str == '0') {
            str++;
            if (*str == 'x' || *str == 'X') {
                base = 16;
                str++;
            } else if (*str == 'b' || *str == 'B') {
                base = 2;
                str++;
            } else {
                base = 8;
            }
        } else {
            base = 10;
        }
    }
    
    /* Validate base parameter */
    if (base < 2 || base > 36) {
        errno = EINVAL;
        if (endptr) *endptr = (char *)start;
        return long_to_long_long(0);
    }
    
    /* 
     * CRITICAL: Parse all numbers as negative values internally.
     * This handles the asymmetric range of 64-bit signed integers:
     * - Positive range: 0 to +9,223,372,036,854,775,807
     * - Negative range: -9,223,372,036,854,775,808 to -1
     * 
     * The negative range is slightly larger, allowing us to parse
     * the critical case of LONG_LONG_MIN (-9223372036854775808).
     */
    while ((digit = char_to_digit(*str, base)) >= 0) {
        /* Check for overflow before updating result */
        if (will_overflow(result, base, digit)) {
            errno = ERANGE;
            if (endptr) *endptr = (char *)str;
            return negative ? LONG_LONG_MIN_VALUE : LONG_LONG_MAX_VALUE;
        }
        
        /* Multiply current result by base */
        result = long_long_mul(result, long_to_long_long(base));
        
        /* Subtract the new digit (working with negative numbers) */
        result = long_long_sub(result, long_to_long_long(digit));
        
        str++;
    }
    
    /* Set endptr to point to the first unparsed character */
    if (endptr) {
        *endptr = (char *)str;
    }
    
    /* Apply sign if positive (negative numbers were parsed as-is) */
    if (!negative) {
        /* Special case: negating LONG_LONG_MIN causes overflow */
        if (long_long_eq(result, LONG_LONG_MIN_VALUE)) {
            errno = ERANGE;
            return LONG_LONG_MAX_VALUE;
        }
        result = long_long_negate(result);
    }
    
    return result;
}
