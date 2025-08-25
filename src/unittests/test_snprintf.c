/*
 * test_snprintf.c - Comprehensive unittest for snprintf and vsnprintf functions
 * 
 * Tests the three-state routing system:
 * - Direct OS call (VSNPrintf compatible)
 * - Preprocessing required (format string rewriting)
 * - Fallback implementation (custom formatting)
 * 
 * Copyright (C) 2024 unix.lib2 project
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdarg.h>

#include "minunit.h"
#include "stdio.h"  /* For our snprintf function declarations */

/* Global test counter */
int tests_run = 0;

/* Test buffer size */
#define TEST_BUFFER_SIZE 256

/* Helper function to compare strings and return descriptive error */
static const char* assert_string_equal(const char* expected, const char* actual, const char* test_name)
{
    static char error_msg[512];
    
    if (strcmp(expected, actual) != 0) {
        snprintf(error_msg, sizeof(error_msg), 
                "%s: Expected '%s', got '%s'", test_name, expected, actual);
        return error_msg;
    }
    return NULL;
}

/* Helper function to compare return values */
static const char* assert_return_value(int expected, int actual, const char* test_name)
{
    static char error_msg[256];
    
    if (expected != actual) {
        snprintf(error_msg, sizeof(error_msg), 
                "%s: Expected return %d, got %d", test_name, expected, actual);
        return error_msg;
    }
    return NULL;
}

/* ===== Test Cases for Direct OS Call (State 0) ===== */

static const char* test_direct_os_call_basic(void)
{
    char buffer[TEST_BUFFER_SIZE];
    int result;
    
    /* Test basic integer formatting - should use VSNPrintf directly */
    result = snprintf(buffer, sizeof(buffer), "%ld %lu %lx", 42L, 123L, 0xFFL);
    mu_assert("Basic integer formatting failed", result == 11);
    mu_assert("Basic integer formatting wrong output", 
              strcmp(buffer, "42 123 ff") == 0);
    
    /* Test string formatting - should use VSNPrintf directly */
    result = snprintf(buffer, sizeof(buffer), "%s", "Hello World");
    mu_assert("String formatting failed", result == 11);
    mu_assert("String formatting wrong output", 
              strcmp(buffer, "Hello World") == 0);
    
    /* Test character formatting - should use VSNPrintf directly */
    result = snprintf(buffer, sizeof(buffer), "%c", 'A');
    mu_assert("Character formatting failed", result == 1);
    mu_assert("Character formatting wrong output", 
              strcmp(buffer, "A") == 0);
    
    return NULL;
}

static const char* test_direct_os_call_with_flags(void)
{
    char buffer[TEST_BUFFER_SIZE];
    int result;
    
    /* Test left justification flag */
    result = snprintf(buffer, sizeof(buffer), "%-10s", "test");
    mu_assert("Left justification failed", result == 10);
    mu_assert("Left justification wrong output", 
              strncmp(buffer, "test      ", 10) == 0);
    
    /* Test zero padding flag */
    result = snprintf(buffer, sizeof(buffer), "%05ld", 42L);
    mu_assert("Zero padding failed", result == 5);
    mu_assert("Zero padding wrong output", 
              strcmp(buffer, "00042") == 0);
    
    return NULL;
}

static const char* test_direct_os_call_width_precision(void)
{
    char buffer[TEST_BUFFER_SIZE];
    int result;
    
    /* Test width specification */
    result = snprintf(buffer, sizeof(buffer), "%10s", "test");
    mu_assert("Width specification failed", result == 10);
    mu_assert("Width specification wrong output", 
              strncmp(buffer, "      test", 10) == 0);
    
    /* Test string precision (truncation) */
    result = snprintf(buffer, sizeof(buffer), "%.2s", "testing");
    mu_assert("String precision failed", result == 2);
    mu_assert("String precision wrong output", 
              strcmp(buffer, "te") == 0);
    
    return NULL;
}

/* ===== Test Cases for Preprocessing Required (State 1) ===== */

static const char* test_preprocessing_required(void)
{
    char buffer[TEST_BUFFER_SIZE];
    int result;
    
    /* Test %d without 'l' - should be rewritten to %ld */
    result = snprintf(buffer, sizeof(buffer), "%d", 42);
    mu_assert("Preprocessing %d failed", result == 2);
    mu_assert("Preprocessing %d wrong output", 
              strcmp(buffer, "42") == 0);
    
    /* Test %u without 'l' - should be rewritten to %lu */
    result = snprintf(buffer, sizeof(buffer), "%u", 123);
    mu_assert("Preprocessing %u failed", result == 3);
    mu_assert("Preprocessing %u wrong output", 
              strcmp(buffer, "123") == 0);
    
    /* Test %x without 'l' - should be rewritten to %lx */
    result = snprintf(buffer, sizeof(buffer), "%x", 0xFF);
    mu_assert("Preprocessing %x failed", result == 2);
    mu_assert("Preprocessing %x wrong output", 
              strcmp(buffer, "ff") == 0);
    
    /* Test %c without 'l' - should be rewritten to %lc */
    result = snprintf(buffer, sizeof(buffer), "%c", 'A');
    mu_assert("Preprocessing %c failed", result == 1);
    mu_assert("Preprocessing %c wrong output", 
              strcmp(buffer, "A") == 0);
    
    return NULL;
}

static const char* test_preprocessing_mixed_formats(void)
{
    char buffer[TEST_BUFFER_SIZE];
    int result;
    
    /* Test mixed formats - some need preprocessing, some don't */
    result = snprintf(buffer, sizeof(buffer), "%d %ld %u %lu", 42, 123L, 456, 789L);
    mu_assert("Mixed preprocessing failed", result == 13);
    mu_assert("Mixed preprocessing wrong output", 
              strcmp(buffer, "42 123 456 789") == 0);
    
    return NULL;
}

/* ===== Test Cases for Fallback Implementation (State 2) ===== */

static const char* test_fallback_floating_point(void)
{
    char buffer[TEST_BUFFER_SIZE];
    int result;
    
    /* Test %f - should use fallback implementation */
    result = snprintf(buffer, sizeof(buffer), "%f", 3.14159);
    mu_assert("Floating point %f failed", result > 0);
    mu_assert("Floating point %f wrong output", 
              strstr(buffer, "3.14") != NULL);
    
    /* Test %e - should use fallback implementation */
    result = snprintf(buffer, sizeof(buffer), "%e", 1234.56);
    mu_assert("Floating point %e failed", result > 0);
    mu_assert("Floating point %e wrong output", 
              strstr(buffer, "1.23") != NULL);
    
    /* Test %g - should use fallback implementation */
    result = snprintf(buffer, sizeof(buffer), "%g", 0.00123);
    mu_assert("Floating point %g failed", result > 0);
    
    return NULL;
}

static const char* test_fallback_octal(void)
{
    char buffer[TEST_BUFFER_SIZE];
    int result;
    
    /* Test %o - should use fallback implementation */
    result = snprintf(buffer, sizeof(buffer), "%o", 42);
    mu_assert("Octal %o failed", result > 0);
    mu_assert("Octal %o wrong output", 
              strcmp(buffer, "52") == 0);  /* 42 decimal = 52 octal */
    
    /* Test %o with # flag */
    result = snprintf(buffer, sizeof(buffer), "%#o", 42);
    mu_assert("Octal %#o failed", result > 0);
    mu_assert("Octal %#o wrong output", 
              strcmp(buffer, "052") == 0);  /* With 0 prefix */
    
    return NULL;
}

static const char* test_fallback_pointer(void)
{
    char buffer[TEST_BUFFER_SIZE];
    int result;
    void *ptr;
    
    ptr = (void*)0x12345678;
    
    /* Test %p - should use fallback implementation */
    result = snprintf(buffer, sizeof(buffer), "%p", ptr);
    mu_assert("Pointer %p failed", result > 0);
    mu_assert("Pointer %p wrong output", 
              strstr(buffer, "0x") != NULL);
    
    return NULL;
}

static const char* test_fallback_precision(void)
{
    char buffer[TEST_BUFFER_SIZE];
    int result;
    
    /* Test floating point with precision */
    result = snprintf(buffer, sizeof(buffer), "%.2f", 3.14159);
    mu_assert("Floating point precision failed", result > 0);
    mu_assert("Floating point precision wrong output", 
              strstr(buffer, "3.14") != NULL);
    
    return NULL;
}

/* ===== Test Cases for Edge Cases and Error Conditions ===== */

static const char* test_edge_cases(void)
{
    char buffer[TEST_BUFFER_SIZE];
    int result;
    
    /* Test NULL buffer with zero size */
    result = snprintf(NULL, 0, "test");
    mu_assert("NULL buffer with zero size failed", result >= 0);
    
    /* Test NULL buffer with non-zero size */
    result = snprintf(NULL, 10, "test");
    mu_assert("NULL buffer with non-zero size should fail", result == -1);
    
    /* Test zero buffer size */
    result = snprintf(buffer, 0, "test");
    mu_assert("Zero buffer size failed", result >= 0);
    
    /* Test buffer overflow protection */
    result = snprintf(buffer, 5, "Hello World");
    mu_assert("Buffer overflow protection failed", result == 11);
    mu_assert("Buffer overflow protection wrong output", 
              strcmp(buffer, "Hell") == 0);  /* Should truncate to 4 chars + NUL */
    
    return NULL;
}

static const char* test_special_values(void)
{
    char buffer[TEST_BUFFER_SIZE];
    int result;
    
    /* Test NaN */
    result = snprintf(buffer, sizeof(buffer), "%f", NAN);
    mu_assert("NaN handling failed", result > 0);
    mu_assert("NaN handling wrong output", 
              strstr(buffer, "nan") != NULL);
    
    /* Test infinity */
    result = snprintf(buffer, sizeof(buffer), "%f", INFINITY);
    mu_assert("Infinity handling failed", result > 0);
    mu_assert("Infinity handling wrong output", 
              strstr(buffer, "inf") != NULL);
    
    /* Test negative infinity */
    result = snprintf(buffer, sizeof(buffer), "%f", -INFINITY);
    mu_assert("Negative infinity handling failed", result > 0);
    mu_assert("Negative infinity handling wrong output", 
              strstr(buffer, "-inf") != NULL);
    
    return NULL;
}

/* ===== Test Cases for Direct Fallback Functions ===== */

static const char* test_direct_fallback_functions(void)
{
    char buffer[TEST_BUFFER_SIZE];
    int result;
    
    /* Test __snprintf directly */
    result = __snprintf(buffer, sizeof(buffer), "%f %o %p", 3.14, 42, (void*)0x12345678);
    mu_assert("Direct __snprintf failed", result > 0);
    
    /* Test __vsnprintf directly */
    result = __vsnprintf(buffer, sizeof(buffer), "%f %o %p", 3.14, 42, (void*)0x12345678);
    mu_assert("Direct __vsnprintf failed", result > 0);
    
    return NULL;
}

/* ===== Test Cases for vsnprintf ===== */

static const char* test_vsnprintf_functionality(void)
{
    char buffer[TEST_BUFFER_SIZE];
    int result;
    va_list args;
    
    /* Test vsnprintf with variadic arguments */
    va_start(args, "test");
    result = vsnprintf(buffer, sizeof(buffer), "%d %s", 42, "test");
    va_end(args);
    mu_assert("vsnprintf failed", result > 0);
    
    return NULL;
}

/* ===== Test Runner ===== */

static const char* all_tests(void)
{
    /* Direct OS Call tests */
    mu_run_test(test_direct_os_call_basic);
    mu_run_test(test_direct_os_call_with_flags);
    mu_run_test(test_direct_os_call_width_precision);
    
    /* Preprocessing required tests */
    mu_run_test(test_preprocessing_required);
    mu_run_test(test_preprocessing_mixed_formats);
    
    /* Fallback implementation tests */
    mu_run_test(test_fallback_floating_point);
    mu_run_test(test_fallback_octal);
    mu_run_test(test_fallback_pointer);
    mu_run_test(test_fallback_precision);
    
    /* Edge cases and error conditions */
    mu_run_test(test_edge_cases);
    mu_run_test(test_special_values);
    
    /* Direct fallback function tests */
    mu_run_test(test_direct_fallback_functions);
    
    /* vsnprintf tests */
    mu_run_test(test_vsnprintf_functionality);
    
    return NULL;
}

/* ===== Main Function ===== */

int main(void)
{
    const char *result;
    
    printf("Testing snprintf and vsnprintf functions...\n\n");
    
    result = all_tests();
    
    if (result != NULL) {
        printf("FAILURE: %s\n", result);
    } else {
        printf("SUCCESS: All tests passed!\n");
    }
    
    printf("Tests run: %d\n", tests_run);
    
    return result != NULL ? 1 : 0;
}
