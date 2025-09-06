/*
 * SPDX-License-Identifier: BSD-2-Clause
 * atol.c - convert string to long integer
 *
 * This function converts a string to a long integer, skipping leading
 * whitespace and handling optional sign.
 *
 * Copyright (C) 2025 by amigazen project
 */

#include <stdlib.h>
#include <ctype.h>
#include <stddef.h>

/**
 * @brief Convert string to long integer
 * @param number Pointer to the string to convert
 * @return Long integer value
 * 
 * The atol() function converts the initial portion of the string pointed to
 * by 'number' to a long integer representation. It skips leading whitespace
 * and handles an optional '+' or '-' sign.
 * 
 * This implementation:
 * - Skips leading whitespace characters
 * - Handles optional '+' or '-' sign
 * - Converts decimal digits to long integer
 * - Returns 0 for invalid input
 * - Is C89 compliant for SAS/C compiler compatibility
 */
long atol(const char *number)
{
    long n;
    int neg;
    
    if (number == NULL) {
        return 0;
    }
    
    n = 0;
    neg = 0;
    
    /* Skip leading whitespace */
    while (isspace(*number)) {
        ++number;
    }
    
    /* Handle optional sign */
    if (*number == '-') {
        neg = 1;
        ++number;
    } else if (*number == '+') {
        ++number;
    }
    
    /* Convert digits */
    while (isdigit(*number)) {
        n = (n * 10) + ((*number++) - '0');
    }
    
    return neg ? -n : n;
}

/**
 * @brief Convert string to integer
 * @param number Pointer to the string to convert
 * @return Integer value
 * 
 * The atoi() function converts the initial portion of the string pointed to
 * by 'number' to an integer representation. It uses atol() internally.
 * 
 * This implementation:
 * - Uses atol() for the conversion
 * - Casts result to int
 */
int atoi(const char *number)
{
    return (int)atol(number);
}
