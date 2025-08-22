/*
 * minunit.h - A minimal unit testing framework for C
 * 
 * This is a header-only testing framework that provides simple macros
 * for writing unit tests. It's designed to be C89 compatible and
 * lightweight for embedded and legacy systems.
 */

#ifndef MINUNIT_H
#define MINUNIT_H

#include <stdio.h>

/* Test assertion macro */
#define mu_assert(message, test) do { if (!(test)) return message; } while (0)

/* Test runner macro */
#define mu_run_test(test) do { \
    const char *message = test(); \
    tests_run++; \
    if (message) return message; \
} while (0)

/* Global test counter */
extern int tests_run;

/* Test result type */
typedef const char* test_result_t;

/* Test function type */
typedef test_result_t (*test_function_t)(void);

#endif /* MINUNIT_H */
