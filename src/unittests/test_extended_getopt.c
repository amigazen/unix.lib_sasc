/*
 * SPDX-License-Identifier: BSD-2-Clause
 * test_extended_getopt.c - Test program for extended getopt functionality
 *
 * This program tests both the basic getopt() function and the extended
 * getopt_long() and getopt_long_only() functions for long option support.
 *
 * Copyright (C) 2025 amigazen project
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Include our extended getopt header */
#include "/include/getopt.h"

/*
 * Test basic getopt() function
 */
void test_basic_getopt(int argc, char *argv[])
{
    int c;
    int verbose = 0;
    char *input_file = NULL;
    char *output_file = NULL;
    
    printf("=== Testing basic getopt() function ===\n");
    printf("Command line arguments: ");
    for (int i = 0; i < argc; i++) {
        printf("%s ", argv[i]);
    }
    printf("\n");
    
    /* Reset getopt state */
    optind = 1;
    
    /* Parse command line options */
    while ((c = getopt(argc, argv, "vi:o:")) != EOF) {
        switch (c) {
            case 'v':
                verbose = 1;
                printf("Verbose mode enabled\n");
                break;
            case 'i':
                input_file = optarg;
                printf("Input file: %s\n", input_file);
                break;
            case 'o':
                output_file = optarg;
                printf("Output file: %s\n", output_file);
                break;
            case '?':
                printf("Unknown option: %c\n", optopt);
                break;
            default:
                printf("Unexpected option: %c\n", c);
                break;
        }
    }
    
    /* Print remaining arguments */
    if (optind < argc) {
        printf("Non-option arguments: ");
        while (optind < argc) {
            printf("%s ", argv[optind++]);
        }
        printf("\n");
    }
    
    printf("Basic getopt() test completed successfully\n\n");
}

/*
 * Test getopt_long() function
 */
void test_getopt_long(int argc, char *argv[])
{
    int c;
    int verbose = 0;
    char *input_file = NULL;
    char *output_file = NULL;
    int create_flag = 0;
    int append_flag = 0;
    
    printf("=== Testing getopt_long() function ===\n");
    printf("Command line arguments: ");
    for (int i = 0; i < argc; i++) {
        printf("%s ", argv[i]);
    }
    printf("\n");
    
    /* Define long options */
    static struct option long_options[] = {
        {"verbose", no_argument, 0, 'v'},
        {"input", required_argument, 0, 'i'},
        {"output", required_argument, 0, 'o'},
        {"create", no_argument, &create_flag, 1},
        {"append", no_argument, &append_flag, 1},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    
    /* Reset getopt state */
    optind = 1;
    
    /* Parse command line options with long option support */
    while ((c = getopt_long(argc, argv, "vi:o:h", long_options, NULL)) != EOF) {
        switch (c) {
            case 'v':
                verbose = 1;
                printf("Verbose mode enabled\n");
                break;
            case 'i':
                input_file = optarg;
                printf("Input file: %s\n", input_file);
                break;
            case 'o':
                output_file = optarg;
                printf("Output file: %s\n", output_file);
                break;
            case 'h':
                printf("Help requested\n");
                break;
            case 0:
                /* Long option with flag set */
                printf("Long option flag set\n");
                break;
            case '?':
                printf("Unknown option: %c\n", optopt);
                break;
            default:
                printf("Unexpected option: %c\n", c);
                break;
        }
    }
    
    /* Check flags set by long options */
    if (create_flag) {
        printf("Create flag is set\n");
    }
    if (append_flag) {
        printf("Append flag is set\n");
    }
    
    /* Print remaining arguments */
    if (optind < argc) {
        printf("Non-option arguments: ");
        while (optind < argc) {
            printf("%s ", argv[optind++]);
        }
        printf("\n");
    }
    
    printf("getopt_long() test completed successfully\n\n");
}

/*
 * Test getopt_long_only() function
 */
void test_getopt_long_only(int argc, char *argv[])
{
    int c;
    int verbose = 0;
    char *input_file = NULL;
    
    printf("=== Testing getopt_long_only() function ===\n");
    printf("Command line arguments: ");
    for (int i = 0; i < argc; i++) {
        printf("%s ", argv[i]);
    }
    printf("\n");
    
    /* Define long options */
    static struct option long_options[] = {
        {"verbose", no_argument, 0, 'v'},
        {"input", required_argument, 0, 'i'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    
    /* Reset getopt state */
    optind = 1;
    
    /* Parse command line options with long-only option support */
    while ((c = getopt_long_only(argc, argv, "vi:h", long_options, NULL)) != EOF) {
        switch (c) {
            case 'v':
                verbose = 1;
                printf("Verbose mode enabled\n");
                break;
            case 'i':
                input_file = optarg;
                printf("Input file: %s\n", input_file);
                break;
            case 'h':
                printf("Help requested\n");
                break;
            case '?':
                printf("Unknown option\n");
                break;
            default:
                printf("Unexpected option: %c\n", c);
                break;
        }
    }
    
    /* Print remaining arguments */
    if (optind < argc) {
        printf("Non-option arguments: ");
        while (optind < argc) {
            printf("%s ", argv[optind++]);
        }
        printf("\n");
    }
    
    printf("getopt_long_only() test completed successfully\n\n");
}

/*
 * Main function
 */
int main(int argc, char *argv[])
{
    printf("=== Testing Extended getopt Functionality ===\n\n");
    
    /* Test basic getopt() */
    test_basic_getopt(argc, argv);
    
    /* Test getopt_long() */
    test_getopt_long(argc, argv);
    
    /* Test getopt_long_only() */
    test_getopt_long_only(argc, argv);
    
    printf("=== All Extended getopt Tests Completed ===\n");
    printf("\nUsage examples:\n");
    printf("  Basic options: %s -v -i input.txt -o output.txt\n", argv[0]);
    printf("  Long options:  %s --verbose --input=input.txt --output=output.txt\n", argv[0]);
    printf("  Mixed options: %s -v --input=input.txt --create\n", argv[0]);
    
    return 0;
}
