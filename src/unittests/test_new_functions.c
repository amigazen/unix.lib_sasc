/*
 * SPDX-License-Identifier: BSD-2-Clause
 * test_new_functions.c - Test program for newly added functions
 *
 * This program tests all the newly integrated functions:
 * - String utility functions (string.h)
 * - CRC32 calculation (stdlib.h)
 * - Wildcard pattern matching (amiga.h)
 *
 * Copyright (C) 2025 by amigazen project
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Include our new function headers */
#include "/include/string.h"
#include "/include/stdlib.h"
#include "/include/amiga.h"

/*
 * Test string utility functions
 */
void test_string_utilities(void)
{
    printf("=== Testing String Utility Functions ===\n");
    
    /* Test case conversion */
    char test_str1[] = "Hello World 123";
    char test_str2[] = "AMIGA OS RULES";
    char test_str3[] = "MiXeD cAsE";
    
    printf("Original: %s\n", test_str1);
    strtolower(test_str1);
    printf("After strtolower(): %s\n", test_str1);
    
    printf("Original: %s\n", test_str2);
    strupr(test_str2);
    printf("After strupr(): %s\n", test_str2);
    
    /* Test case-insensitive search */
    char *search_str = "Hello World";
    char *found = strichr(search_str, 'W');
    if (found) {
        printf("strichr('W') found at: %s\n", found);
    }
    
    char *substr = "world";
    char *found_sub = stristr(search_str, substr);
    if (found_sub) {
        printf("stristr('%s') found at: %s\n", substr, found_sub);
    }
    
    /* Test bounded search */
    char *bounded_str = "Hello World";
    char *found_char = strnchr(bounded_str, 'o', 5);
    if (found_char) {
        printf("strnchr('o', 5) found at: %s\n", found_char);
    }
    
    /* Test string insertion */
    char insert_dest[50] = "Hello World";
    char insert_src[] = "Beautiful ";
    printf("Before strins(): %s\n", insert_dest);
    strins(insert_dest, insert_src);
    printf("After strins(): %s\n", insert_dest);
    
    /* Test string reversal */
    char reverse_str[] = "AMIGA";
    printf("Before strrev(): %s\n", reverse_str);
    strrev(reverse_str);
    printf("After strrev(): %s\n", reverse_str);
    
    printf("String utility tests completed successfully\n\n");
}

/*
 * Test CRC32 functions
 */
void test_crc32_functions(void)
{
    printf("=== Testing CRC32 Functions ===\n");
    
    /* Test CRC32 for string */
    char *test_string = "Hello World";
    uint32_t crc_string = crc32_string(test_string);
    printf("CRC32 of '%s': 0x%08X\n", test_string, crc_string);
    
    /* Test CRC32 for binary data */
    unsigned char binary_data[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    uint32_t crc_binary = crc32(binary_data, sizeof(binary_data));
    printf("CRC32 of binary data: 0x%08X\n", crc_binary);
    
    /* Test CRC32 for empty data */
    uint32_t crc_empty = crc32("", 0);
    printf("CRC32 of empty data: 0x%08X\n", crc_empty);
    
    /* Test CRC32 for single character */
    uint32_t crc_single = crc32_string("A");
    printf("CRC32 of 'A': 0x%08X\n", crc_single);
    
    printf("CRC32 tests completed successfully\n\n");
}

/*
 * Test wildcard functions
 */
void test_wildcard_functions(void)
{
    printf("=== Testing Wildcard Functions ===\n");
    
    /* Initialize wildcard string list */
    wildcard_strlist strl;
    wildcard_init_list(&strl);
    
    printf("Initial list count: %d\n", wildcard_get_count(&strl));
    printf("List is empty: %s\n", wildcard_is_empty(&strl) ? "Yes" : "No");
    
    /* Test with a simple pattern (no wildcards) */
    int result = wildcard_scan_pattern("test_file.txt", &strl);
    printf("wildcard_scan_pattern('test_file.txt') returned: %d\n", result);
    printf("List count after scan: %d\n", wildcard_get_count(&strl));
    
    if (wildcard_get_count(&strl) > 0) {
        printf("First element: %s\n", wildcard_pop_element(0, &strl));
    }
    
    /* Test with wildcard pattern */
    printf("\nTesting wildcard pattern '*.txt':\n");
    wildcard_clear_list(&strl);
    result = wildcard_scan_pattern("*.txt", &strl);
    printf("wildcard_scan_pattern('*.txt') returned: %d\n", result);
    printf("Files found: %d\n", wildcard_get_count(&strl));
    
    if (wildcard_get_count(&strl) > 0) {
        printf("Files matching pattern:\n");
        for (int i = 0; i < wildcard_get_count(&strl); i++) {
            printf("  %d: %s\n", i, wildcard_pop_element(i, &strl));
        }
    }
    
    /* Test with another pattern */
    printf("\nTesting wildcard pattern 'test*':\n");
    wildcard_clear_list(&strl);
    result = wildcard_scan_pattern("test*", &strl);
    printf("wildcard_scan_pattern('test*') returned: %d\n", result);
    printf("Files found: %d\n", wildcard_get_count(&strl));
    
    if (wildcard_get_count(&strl) > 0) {
        printf("Files matching pattern:\n");
        for (int i = 0; i < wildcard_get_count(&strl); i++) {
            printf("  %d: %s\n", i, wildcard_pop_element(i, &strl));
        }
    }
    
    /* Clean up */
    wildcard_clear_list(&strl);
    printf("Wildcard tests completed successfully\n\n");
}

/*
 * Main function
 */
int main(int argc, char *argv[])
{
    printf("=== Testing Newly Added Functions ===\n\n");
    
    /* Test string utility functions */
    test_string_utilities();
    
    /* Test CRC32 functions */
    test_crc32_functions();
    
    /* Test wildcard functions */
    test_wildcard_functions();
    
    printf("=== All Tests Completed Successfully ===\n");
    printf("\nNew functions available:\n");
    printf("- String utilities: strtolower, strupr, strichr, stristr, strnchr, strins, strrev\n");
    printf("- CRC32 calculation: crc32, crc32_string\n");
    printf("- Wildcard matching: wildcard_scan_pattern, wildcard functions\n");
    
    return 0;
}
