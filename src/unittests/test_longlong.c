#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include "minunit.h"
#include "/longlong.h"

/* Global test counter */
int tests_run = 0;

/* Function prototypes for all test functions */
static const char* test_long_to_long_long(void);
static const char* test_get_long_long_max(void);
static const char* test_get_long_long_min(void);
static const char* test_add_simple(void);
static const char* test_add_carry(void);
static const char* test_add_negative(void);
static const char* test_sub_simple(void);
static const char* test_sub_borrow(void);
static const char* test_mul_simple(void);
static const char* test_mul_cross_boundary(void);
static const char* test_div_simple(void);
static const char* test_eq_true(void);
static const char* test_eq_false(void);
static const char* test_lt_true(void);
static const char* test_lt_false(void);
static const char* test_strtoll_basic(void);
static const char* test_strtoll_negative(void);
static const char* test_strtoll_hex(void);
static const char* test_strtoll_auto_base(void);
static const char* test_strtoll_llmin(void);
static const char* test_strtoll_overflow(void);
static const char* test_strtoll_invalid_base(void);
static const char* all_tests(void);

/* Helper function to create long_long_t values for C89 compatibility */
long_long_t make_long_long(unsigned long hi, unsigned long lo)
{
    long_long_t result;
    result.hi = hi;
    result.lo = lo;
    return result;
}

/* Helper function to compare long_long_t values */
int ll_eq(long_long_t a, long_long_t b)
{
    return (a.hi == b.hi && a.lo == b.lo);
}

/* Helper function to print long_long_t values for debugging */
void print_ll(const char *label, long_long_t value)
{
    printf("%s: hi=0x%08lX, lo=0x%08lX\n", label, value.hi, value.lo);
}

/* ===== Test Cases for Basic Functions ===== */

static const char* test_long_to_long_long(void)
{
    long_long_t result;
    long_long_t expected;
    
    result = long_to_long_long(12345);
    expected = make_long_long(0, 12345);
    mu_assert("long_to_long_long: failed to convert positive number", ll_eq(result, expected));
    
    result = long_to_long_long(-12345);
    expected = make_long_long(0xFFFFFFFF, 0xFFFFCEC7); /* Two's complement */
    mu_assert("long_to_long_long: failed to convert negative number", ll_eq(result, expected));
    
    return 0;
}

static const char* test_get_long_long_max(void)
{
    long_long_t result;
    long_long_t expected;
    
    result = get_long_long_max();
    expected = make_long_long(0x7FFFFFFF, 0xFFFFFFFF);
    mu_assert("get_long_long_max: returned incorrect value", ll_eq(result, expected));
    return 0;
}

static const char* test_get_long_long_min(void)
{
    long_long_t result;
    long_long_t expected;
    
    result = get_long_long_min();
    expected = make_long_long(0x80000000, 0x00000000);
    mu_assert("get_long_long_min: returned incorrect value", ll_eq(result, expected));
    return 0;
}

/* ===== Test Cases for Addition ===== */

static const char* test_add_simple(void)
{
    long_long_t a;
    long_long_t b;
    long_long_t result;
    long_long_t expected;
    
    a = make_long_long(0, 5);
    b = make_long_long(0, 10);
    result = long_long_add(a, b);
    expected = make_long_long(0, 15);
    mu_assert("add_simple: 5 + 10 != 15", ll_eq(result, expected));
    return 0;
}

static const char* test_add_carry(void)
{
    long_long_t a;
    long_long_t b;
    long_long_t result;
    long_long_t expected;
    
    a = make_long_long(0, 0xFFFFFFFF); /* Max 32-bit unsigned */
    b = make_long_long(0, 1);
    result = long_long_add(a, b);
    expected = make_long_long(1, 0);
    mu_assert("add_carry: failed to carry from .lo to .hi", ll_eq(result, expected));
    return 0;
}

static const char* test_add_negative(void)
{
    long_long_t a;
    long_long_t b;
    long_long_t result;
    long_long_t expected;
    
    a = make_long_long(0, 10);
    b = make_long_long(0xFFFFFFFF, 0xFFFFFFF6); /* -10 */
    result = long_long_add(a, b);
    expected = make_long_long(0, 0);
    mu_assert("add_negative: 10 + (-10) != 0", ll_eq(result, expected));
    return 0;
}

/* ===== Test Cases for Subtraction ===== */

static const char* test_sub_simple(void)
{
    long_long_t a;
    long_long_t b;
    long_long_t result;
    long_long_t expected;
    
    a = make_long_long(0, 15);
    b = make_long_long(0, 10);
    result = long_long_sub(a, b);
    expected = make_long_long(0, 5);
    mu_assert("sub_simple: 15 - 10 != 5", ll_eq(result, expected));
    return 0;
}

static const char* test_sub_borrow(void)
{
    long_long_t a;
    long_long_t b;
    long_long_t result;
    long_long_t expected;
    
    a = make_long_long(1, 0);
    b = make_long_long(0, 1);
    result = long_long_sub(a, b);
    expected = make_long_long(0, 0xFFFFFFFF);
    mu_assert("sub_borrow: failed to borrow from .hi to .lo", ll_eq(result, expected));
    return 0;
}

/* ===== Test Cases for Multiplication ===== */

static const char* test_mul_simple(void)
{
    long_long_t a;
    long_long_t b;
    long_long_t result;
    long_long_t expected;
    
    a = make_long_long(0, 5);
    b = make_long_long(0, 10);
    result = long_long_mul(a, b);
    expected = make_long_long(0, 50);
    mu_assert("mul_simple: 5 * 10 != 50", ll_eq(result, expected));
    return 0;
}

static const char* test_mul_cross_boundary(void)
{
    long_long_t a;
    long_long_t b;
    long_long_t result;
    long_long_t expected;
    
    a = make_long_long(0, 0x80000000); /* 2^31 */
    b = make_long_long(0, 2);
    result = long_long_mul(a, b);
    expected = make_long_long(1, 0); /* 2^32 */
    mu_assert("mul_cross_boundary: failed to cross 32-bit boundary", ll_eq(result, expected));
    return 0;
}

/* ===== Test Cases for Division ===== */

static const char* test_div_simple(void)
{
    long_long_t a;
    long_long_t b;
    long_long_t result;
    long_long_t expected;
    
    a = make_long_long(0, 50);
    b = make_long_long(0, 10);
    result = long_long_div(a, b);
    expected = make_long_long(0, 5);
    mu_assert("div_simple: 50 / 10 != 5", ll_eq(result, expected));
    return 0;
}

/* ===== Test Cases for Comparison Functions ===== */

static const char* test_eq_true(void)
{
    long_long_t a;
    long_long_t b;
    
    a = make_long_long(0, 12345);
    b = make_long_long(0, 12345);
    mu_assert("eq_true: equal values should return true", long_long_eq(a, b));
    return 0;
}

static const char* test_eq_false(void)
{
    long_long_t a;
    long_long_t b;
    
    a = make_long_long(0, 12345);
    b = make_long_long(0, 12346);
    mu_assert("eq_false: different values should return false", !long_long_eq(a, b));
    return 0;
}

static const char* test_lt_true(void)
{
    long_long_t a;
    long_long_t b;
    
    a = make_long_long(0, 12345);
    b = make_long_long(0, 12346);
    mu_assert("lt_true: 12345 < 12346 should return true", long_long_lt(a, b));
    return 0;
}

static const char* test_lt_false(void)
{
    long_long_t a;
    long_long_t b;
    
    a = make_long_long(0, 12346);
    b = make_long_long(0, 12345);
    mu_assert("lt_false: 12346 < 12345 should return false", !long_long_lt(a, b));
    return 0;
}

/* ===== Test Cases for strtoll ===== */

static const char* test_strtoll_basic(void)
{
    char *endptr;
    long_long_t expected;
    long_long_t result;
    
    expected = make_long_long(0, 12345);
    result = strtoll("12345", &endptr, 10);
    mu_assert("strtoll_basic: failed to parse '12345'", ll_eq(result, expected));
    mu_assert("strtoll_basic: endptr is not empty", *endptr == '\0');
    return 0;
}

static const char* test_strtoll_negative(void)
{
    char *endptr;
    long_long_t expected;
    long_long_t result;
    
    expected = make_long_long(0xFFFFFFFF, 0xFFFFCEC7); /* -12345 */
    result = strtoll("-12345", &endptr, 10);
    mu_assert("strtoll_negative: failed to parse '-12345'", ll_eq(result, expected));
    return 0;
}

static const char* test_strtoll_hex(void)
{
    char *endptr;
    long_long_t expected;
    long_long_t result;
    
    expected = make_long_long(0, 0x1A2B3C4D);
    result = strtoll("1A2B3C4D", &endptr, 16);
    mu_assert("strtoll_hex: failed to parse hex '1A2B3C4D'", ll_eq(result, expected));
    return 0;
}

static const char* test_strtoll_auto_base(void)
{
    char *endptr;
    long_long_t expected;
    long_long_t result;
    
    expected = make_long_long(0, 0x12345678);
    result = strtoll("0x12345678", &endptr, 0);
    mu_assert("strtoll_auto_base: failed to auto-detect hex base", ll_eq(result, expected));
    return 0;
}

static const char* test_strtoll_llmin(void)
{
    char *endptr;
    long_long_t expected;
    long_long_t result;
    
    expected = make_long_long(0x80000000, 0x00000000); /* LONG_LONG_MIN */
    result = strtoll("-9223372036854775808", &endptr, 10);
    mu_assert("strtoll_llmin: failed to parse LONG_LONG_MIN", ll_eq(result, expected));
    mu_assert("strtoll_llmin: errno should be 0", errno == 0);
    return 0;
}

static const char* test_strtoll_overflow(void)
{
    char *endptr;
    long_long_t result;
    
    errno = 0;
    result = strtoll("9223372036854775808", &endptr, 10);
    mu_assert("strtoll_overflow: should set errno to ERANGE", errno == ERANGE);
    return 0;
}

static const char* test_strtoll_invalid_base(void)
{
    char *endptr;
    long_long_t result;
    
    errno = 0;
    result = strtoll("123", &endptr, 37);
    mu_assert("strtoll_invalid_base: should set errno to EINVAL", errno == EINVAL);
    return 0;
}

/* ===== Test Runner ===== */

static const char* all_tests(void)
{
    /* Basic functions */
    mu_run_test(test_long_to_long_long);
    mu_run_test(test_get_long_long_max);
    mu_run_test(test_get_long_long_min);
    
    /* Arithmetic functions */
    mu_run_test(test_add_simple);
    mu_run_test(test_add_carry);
    mu_run_test(test_add_negative);
    mu_run_test(test_sub_simple);
    mu_run_test(test_sub_borrow);
    mu_run_test(test_mul_simple);
    mu_run_test(test_mul_cross_boundary);
    mu_run_test(test_div_simple);
    
    /* Comparison functions */
    mu_run_test(test_eq_true);
    mu_run_test(test_eq_false);
    mu_run_test(test_lt_true);
    mu_run_test(test_lt_false);
    
    /* String conversion functions */
    mu_run_test(test_strtoll_basic);
    mu_run_test(test_strtoll_negative);
    mu_run_test(test_strtoll_hex);
    mu_run_test(test_strtoll_auto_base);
    mu_run_test(test_strtoll_llmin);
    mu_run_test(test_strtoll_overflow);
    mu_run_test(test_strtoll_invalid_base);
    
    return 0;
}

int main(void)
{
    const char *result;
    
    printf("Running longlong library tests...\n\n");
    
    result = all_tests();
    
    if (result != 0) {
        printf("FAILED: %s\n", result);
    } else {
        printf("ALL TESTS PASSED\n");
    }
    
    printf("Tests run: %d\n", tests_run);
    
    return result != 0;
}
