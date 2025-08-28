/*
 * SPDX-License-Identifier: BSD-2-Clause
 * test_getopt_ustat.c - Test program for getopt() and ustat() functions
 *
 * This program tests both the getopt() command line option parsing
 * and the ustat() file system statistics functions.
 *
 * Copyright (C) 2025 amigazen project
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

/* Include our function headers */
#include "/include/getopt.h"
#include "/include/ustat.h"

/*
 * Test getopt() function
 */
void test_getopt(int argc, char *argv[])
{
    int c;
    int verbose = 0;
    char *input_file = NULL;
    char *output_file = NULL;
    
    printf("Testing getopt() function...\n");
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
    
    printf("getopt() test completed successfully\n\n");
}

/*
 * Test ustat() function
 */
void test_ustat(void)
{
    struct stat stat_buf;
    struct ustat ustat_buf;
    const char *test_file = "test_file";
    
    printf("Testing ustat() function...\n");
    
    /* Create a test file to get its device ID */
    FILE *fp = fopen(test_file, "w");
    if (fp) {
        fprintf(fp, "Test file for ustat() function\n");
        fclose(fp);
        printf("Created test file: %s\n", test_file);
        
        /* Get file status to obtain device ID */
        if (stat(test_file, &stat_buf) == 0) {
            printf("File device ID: %08lx\n", (unsigned long)stat_buf.st_dev);
            
            /* Test ustat() function */
            if (ustat(stat_buf.st_dev, &ustat_buf) == 0) {
                printf("ustat() successful:\n");
                printf("  Free blocks: %ld\n", ustat_buf.f_tinode);
                printf("  Free space: %ld KB\n", ustat_buf.f_tfree);
                printf("  Volume name: %s\n", ustat_buf.f_fname[0] ? ustat_buf.f_fname : "(not available)");
            } else {
                printf("ustat() failed: %s (errno=%d)\n", strerror(errno), errno);
            }
        } else {
            printf("stat() failed: %s (errno=%d)\n", strerror(errno), errno);
        }
        
        /* Clean up test file */
        unlink(test_file);
        printf("Removed test file: %s\n", test_file);
    } else {
        printf("Failed to create test file: %s (errno=%d)\n", strerror(errno), errno);
    }
    
    printf("ustat() test completed\n\n");
}

/*
 * Main function
 */
int main(int argc, char *argv[])
{
    printf("=== Testing getopt() and ustat() functions ===\n\n");
    
    /* Test getopt() function */
    test_getopt(argc, argv);
    
    /* Test ustat() function */
    test_ustat();
    
    printf("=== All tests completed ===\n");
    return 0;
}
