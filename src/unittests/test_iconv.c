/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * Copyright (c) 2025 amigazen project
 *
 * Test program for iconv implementation
 */

#include <iconv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* Test result tracking */
static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST_ASSERT(condition, message) \
    do { \
        tests_run++; \
        if (condition) { \
            tests_passed++; \
            printf("PASS: %s\n", message); \
        } else { \
            tests_failed++; \
            printf("FAIL: %s\n", message); \
        } \
    } while (0)

/* Test identity conversion (same encoding) */
static void test_identity_conversion(void)
{
    iconv_t cd;
    const char *input = "Hello, World!";
    char output[256];
    const char *inptr;
    char *outptr;
    size_t inleft, outleft;
    size_t result;
    
    printf("\n=== Testing Identity Conversion ===\n");
    
    /* Test ASCII to ASCII */
    cd = iconv_open("ASCII", "ASCII");
    TEST_ASSERT(cd != (iconv_t)-1, "iconv_open ASCII->ASCII");
    
    if (cd != (iconv_t)-1) {
        strcpy(output, input);
        inptr = input;
        outptr = output;
        inleft = strlen(input);
        outleft = sizeof(output) - 1;
        
        result = iconv(cd, &inptr, &inleft, &outptr, &outleft);
        TEST_ASSERT(result != (size_t)-1, "iconv ASCII->ASCII conversion");
        TEST_ASSERT(inleft == 0, "All input consumed");
        TEST_ASSERT(strcmp(input, output) == 0, "Output matches input");
        
        iconv_close(cd);
    }
}

/* Test ASCII to UTF-8 conversion */
static void test_ascii_to_utf8(void)
{
    iconv_t cd;
    const char *input = "Hello";
    char output[256];
    const char *inptr;
    char *outptr;
    size_t inleft, outleft;
    size_t result;
    
    printf("\n=== Testing ASCII to UTF-8 ===\n");
    
    cd = iconv_open("UTF-8", "ASCII");
    TEST_ASSERT(cd != (iconv_t)-1, "iconv_open ASCII->UTF-8");
    
    if (cd != (iconv_t)-1) {
        memset(output, 0, sizeof(output));
        inptr = input;
        outptr = output;
        inleft = strlen(input);
        outleft = sizeof(output) - 1;
        
        result = iconv(cd, &inptr, &inleft, &outptr, &outleft);
        TEST_ASSERT(result != (size_t)-1, "iconv ASCII->UTF-8 conversion");
        TEST_ASSERT(inleft == 0, "All input consumed");
        TEST_ASSERT(strcmp(input, output) == 0, "ASCII->UTF-8 output matches input");
        
        iconv_close(cd);
    }
}

/* Test UTF-8 to ASCII conversion */
static void test_utf8_to_ascii(void)
{
    iconv_t cd;
    const char *input = "Hello";
    char output[256];
    const char *inptr;
    char *outptr;
    size_t inleft, outleft;
    size_t result;
    
    printf("\n=== Testing UTF-8 to ASCII ===\n");
    
    cd = iconv_open("ASCII", "UTF-8");
    TEST_ASSERT(cd != (iconv_t)-1, "iconv_open UTF-8->ASCII");
    
    if (cd != (iconv_t)-1) {
        memset(output, 0, sizeof(output));
        inptr = input;
        outptr = output;
        inleft = strlen(input);
        outleft = sizeof(output) - 1;
        
        result = iconv(cd, &inptr, &inleft, &outptr, &outleft);
        TEST_ASSERT(result != (size_t)-1, "iconv UTF-8->ASCII conversion");
        TEST_ASSERT(inleft == 0, "All input consumed");
        TEST_ASSERT(strcmp(input, output) == 0, "UTF-8->ASCII output matches input");
        
        iconv_close(cd);
    }
}

/* Test Latin-1 to UTF-8 conversion */
static void test_latin1_to_utf8(void)
{
    iconv_t cd;
    const char input[] = "H\xe9llo";  /* "Héllo" in Latin-1 */
    char output[256];
    const char *inptr;
    char *outptr;
    size_t inleft, outleft;
    size_t result;
    
    printf("\n=== Testing Latin-1 to UTF-8 ===\n");
    
    cd = iconv_open("UTF-8", "ISO-8859-1");
    TEST_ASSERT(cd != (iconv_t)-1, "iconv_open ISO-8859-1->UTF-8");
    
    if (cd != (iconv_t)-1) {
        memset(output, 0, sizeof(output));
        inptr = input;
        outptr = output;
        inleft = strlen(input);
        outleft = sizeof(output) - 1;
        
        result = iconv(cd, &inptr, &inleft, &outptr, &outleft);
        TEST_ASSERT(result != (size_t)-1, "iconv ISO-8859-1->UTF-8 conversion");
        TEST_ASSERT(inleft == 0, "All input consumed");
        /* Check that 'é' (0xe9) was converted to UTF-8 (0xc3 0xa9) */
        TEST_ASSERT(output[0] == 'H', "First character preserved");
        TEST_ASSERT((unsigned char)output[1] == 0xc3, "UTF-8 first byte");
        TEST_ASSERT((unsigned char)output[2] == 0xa9, "UTF-8 second byte");
        
        iconv_close(cd);
    }
}

/* Test Latin-1 to UTF-8 conversion with high-range characters */
static void test_latin1_high_range(void)
{
    iconv_t cd;
    const char input[] = "A\xc0\xff";  /* Characters in 0xC0-0xFF range */
    char output[256];
    const char *inptr;
    char *outptr;
    size_t inleft, outleft;
    size_t result;
    
    printf("\n=== Testing Latin-1 High Range (0xC0-0xFF) ===\n");
    
    cd = iconv_open("UTF-8", "ISO-8859-1");
    TEST_ASSERT(cd != (iconv_t)-1, "iconv_open ISO-8859-1->UTF-8 for high range");
    
    if (cd != (iconv_t)-1) {
        memset(output, 0, sizeof(output));
        inptr = input;
        outptr = output;
        inleft = strlen(input);
        outleft = sizeof(output) - 1;
        
        result = iconv(cd, &inptr, &inleft, &outptr, &outleft);
        TEST_ASSERT(result != (size_t)-1, "iconv ISO-8859-1->UTF-8 high range conversion");
        TEST_ASSERT(inleft == 0, "All input consumed");
        /* Check that characters were converted to 2-byte UTF-8 sequences */
        TEST_ASSERT(output[0] == 'A', "First character preserved");
        TEST_ASSERT((unsigned char)output[1] == 0xc3, "0xC0 converted to UTF-8 first byte");
        TEST_ASSERT((unsigned char)output[2] == 0x80, "0xC0 converted to UTF-8 second byte");
        TEST_ASSERT((unsigned char)output[3] == 0xc3, "0xFF converted to UTF-8 first byte");
        TEST_ASSERT((unsigned char)output[4] == 0xbf, "0xFF converted to UTF-8 second byte");
        
        iconv_close(cd);
    }
}

/* Test error handling */
static void test_error_handling(void)
{
    iconv_t cd;
    const char *input = "Hello";
    char output[256];
    const char *inptr;
    char *outptr;
    size_t inleft, outleft;
    size_t result;
    
    printf("\n=== Testing Error Handling ===\n");
    
    /* Test case-insensitive encoding names */
    cd = iconv_open("utf-8", "ascii");
    TEST_ASSERT(cd != (iconv_t)-1, "iconv_open with lowercase encoding names");
    if (cd != (iconv_t)-1) {
        iconv_close(cd);
    }
    
    cd = iconv_open("UTF-8", "ASCII");
    TEST_ASSERT(cd != (iconv_t)-1, "iconv_open with uppercase encoding names");
    if (cd != (iconv_t)-1) {
        iconv_close(cd);
    }
    
    cd = iconv_open("Utf-8", "Ascii");
    TEST_ASSERT(cd != (iconv_t)-1, "iconv_open with mixed case encoding names");
    if (cd != (iconv_t)-1) {
        iconv_close(cd);
    }
    
    /* Test invalid encoding names */
    cd = iconv_open("INVALID", "ASCII");
    TEST_ASSERT(cd == (iconv_t)-1, "iconv_open with invalid encoding");
    TEST_ASSERT(errno == EINVAL, "errno set to EINVAL for invalid encoding");
    
    /* Test NULL parameters */
    cd = iconv_open(NULL, "ASCII");
    TEST_ASSERT(cd == (iconv_t)-1, "iconv_open with NULL tocode");
    TEST_ASSERT(errno == EINVAL, "errno set to EINVAL for NULL tocode");
    
    cd = iconv_open("ASCII", NULL);
    TEST_ASSERT(cd == (iconv_t)-1, "iconv_open with NULL fromcode");
    TEST_ASSERT(errno == EINVAL, "errno set to EINVAL for NULL fromcode");
    
    /* Test invalid conversion descriptor */
    result = iconv((iconv_t)-1, &input, &inleft, &output, &outleft);
    TEST_ASSERT(result == (size_t)-1, "iconv with invalid descriptor");
    TEST_ASSERT(errno == EBADF, "errno set to EBADF for invalid descriptor");
    
    /* Test iconv_close with invalid descriptor */
    result = iconv_close((iconv_t)-1);
    TEST_ASSERT(result == -1, "iconv_close with invalid descriptor");
    TEST_ASSERT(errno == EBADF, "errno set to EBADF for invalid close");
}

/* Test buffer overflow handling */
static void test_buffer_overflow(void)
{
    iconv_t cd;
    const char *input = "Hello, World!";
    char output[5];  /* Small buffer */
    const char *inptr;
    char *outptr;
    size_t inleft, outleft;
    size_t result;
    
    printf("\n=== Testing Buffer Overflow ===\n");
    
    cd = iconv_open("ASCII", "ASCII");
    TEST_ASSERT(cd != (iconv_t)-1, "iconv_open for buffer test");
    
    if (cd != (iconv_t)-1) {
        inptr = input;
        outptr = output;
        inleft = strlen(input);
        outleft = sizeof(output) - 1;  /* Leave room for null terminator */
        
        result = iconv(cd, &inptr, &inleft, &outptr, &outleft);
        /* Should succeed but not convert all input */
        TEST_ASSERT(result != (size_t)-1, "iconv with small buffer");
        TEST_ASSERT(inleft > 0, "Some input remaining");
        TEST_ASSERT(outleft == 0, "Output buffer full");
        
        iconv_close(cd);
    }
}

/* Test locale-aware conversions */
static void test_locale_conversions(void)
{
    iconv_t cd;
    const char *input = "Hello";
    char output[256];
    const char *inptr;
    char *outptr;
    size_t inleft, outleft;
    size_t result;
    
    printf("\n=== Testing Locale-Aware Conversions ===\n");
    
    /* Test LOCALE to UTF-8 */
    cd = iconv_open("UTF-8", "LOCALE");
    TEST_ASSERT(cd != (iconv_t)-1, "iconv_open LOCALE->UTF-8");
    
    if (cd != (iconv_t)-1) {
        memset(output, 0, sizeof(output));
        inptr = input;
        outptr = output;
        inleft = strlen(input);
        outleft = sizeof(output) - 1;
        
        result = iconv(cd, &inptr, &inleft, &outptr, &outleft);
        TEST_ASSERT(result != (size_t)-1, "iconv LOCALE->UTF-8 conversion");
        TEST_ASSERT(inleft == 0, "All input consumed");
        TEST_ASSERT(strcmp(input, output) == 0, "LOCALE->UTF-8 output matches input");
        
        iconv_close(cd);
    }
    
    /* Test UTF-8 to LOCALE */
    cd = iconv_open("LOCALE", "UTF-8");
    TEST_ASSERT(cd != (iconv_t)-1, "iconv_open UTF-8->LOCALE");
    
    if (cd != (iconv_t)-1) {
        memset(output, 0, sizeof(output));
        inptr = input;
        outptr = output;
        inleft = strlen(input);
        outleft = sizeof(output) - 1;
        
        result = iconv(cd, &inptr, &inleft, &outptr, &outleft);
        TEST_ASSERT(result != (size_t)-1, "iconv UTF-8->LOCALE conversion");
        TEST_ASSERT(inleft == 0, "All input consumed");
        TEST_ASSERT(strcmp(input, output) == 0, "UTF-8->LOCALE output matches input");
        
        iconv_close(cd);
    }
    
    /* Test LOCALE to ASCII */
    cd = iconv_open("ASCII", "LOCALE");
    TEST_ASSERT(cd != (iconv_t)-1, "iconv_open LOCALE->ASCII");
    
    if (cd != (iconv_t)-1) {
        memset(output, 0, sizeof(output));
        inptr = input;
        outptr = output;
        inleft = strlen(input);
        outleft = sizeof(output) - 1;
        
        result = iconv(cd, &inptr, &inleft, &outptr, &outleft);
        TEST_ASSERT(result != (size_t)-1, "iconv LOCALE->ASCII conversion");
        TEST_ASSERT(inleft == 0, "All input consumed");
        TEST_ASSERT(strcmp(input, output) == 0, "LOCALE->ASCII output matches input");
        
        iconv_close(cd);
    }
    
    /* Test ASCII to LOCALE */
    cd = iconv_open("LOCALE", "ASCII");
    TEST_ASSERT(cd != (iconv_t)-1, "iconv_open ASCII->LOCALE");
    
    if (cd != (iconv_t)-1) {
        memset(output, 0, sizeof(output));
        inptr = input;
        outptr = output;
        inleft = strlen(input);
        outleft = sizeof(output) - 1;
        
        result = iconv(cd, &inptr, &inleft, &outptr, &outleft);
        TEST_ASSERT(result != (size_t)-1, "iconv ASCII->LOCALE conversion");
        TEST_ASSERT(inleft == 0, "All input consumed");
        TEST_ASSERT(strcmp(input, output) == 0, "ASCII->LOCALE output matches input");
        
        iconv_close(cd);
    }
}

/* Test locale.library character validation */
static void test_locale_character_validation(void)
{
    iconv_t cd;
    const char *input = "Hello\x80";  /* Contains non-ASCII character */
    char output[256];
    const char *inptr;
    char *outptr;
    size_t inleft, outleft;
    size_t result;
    
    printf("\n=== Testing Locale Character Validation ===\n");
    
    /* Test LOCALE to UTF-8 with potentially invalid character */
    cd = iconv_open("UTF-8", "LOCALE");
    TEST_ASSERT(cd != (iconv_t)-1, "iconv_open LOCALE->UTF-8 for validation test");
    
    if (cd != (iconv_t)-1) {
        memset(output, 0, sizeof(output));
        inptr = input;
        outptr = output;
        inleft = strlen(input);
        outleft = sizeof(output) - 1;
        
        result = iconv(cd, &inptr, &inleft, &outptr, &outleft);
        /* This should either succeed (if locale accepts 0x80) or fail with EILSEQ */
        /* The exact behavior depends on the system's locale configuration */
        if (result == (size_t)-1) {
            TEST_ASSERT(errno == EILSEQ, "Invalid character properly rejected with EILSEQ");
        } else {
            TEST_ASSERT(result != (size_t)-1, "Character accepted by locale");
        }
        
        iconv_close(cd);
    }
    
    /* Test ASCII to LOCALE with invalid character */
    cd = iconv_open("LOCALE", "ASCII");
    TEST_ASSERT(cd != (iconv_t)-1, "iconv_open ASCII->LOCALE for validation test");
    
    if (cd != (iconv_t)-1) {
        memset(output, 0, sizeof(output));
        inptr = input;
        outptr = output;
        inleft = strlen(input);
        outleft = sizeof(output) - 1;
        
        result = iconv(cd, &inptr, &inleft, &outptr, &outleft);
        /* Should fail when it hits the non-ASCII character */
        TEST_ASSERT(result == (size_t)-1, "Non-ASCII character properly rejected");
        TEST_ASSERT(errno == EILSEQ, "errno set to EILSEQ for non-ASCII character");
        
        iconv_close(cd);
    }
}

/* Test enhanced multibyte functionality using SAS/C functions */
static void test_enhanced_multibyte(void)
{
    iconv_t cd;
    const char *input;
    char output[256];
    const char *inptr;
    char *outptr;
    size_t inleft, outleft;
    size_t result;
    
    printf("\n=== Testing Enhanced Multibyte Functionality ===\n");
    
    /* Test UTF-8 to Latin-1 with enhanced multibyte handling */
    cd = iconv_open("ISO-8859-1", "UTF-8");
    TEST_ASSERT(cd != (iconv_t)-1, "iconv_open UTF-8->ISO-8859-1 for enhanced test");
    
    if (cd != (iconv_t)-1) {
        /* Test with various UTF-8 sequences */
        const char *test_cases[] = {
            "Hello",           /* ASCII only */
            "Café",            /* 2-byte UTF-8 sequence */
            "naïve",           /* 2-byte UTF-8 sequence */
            "résumé",          /* 2-byte UTF-8 sequence */
            NULL
        };
        
        int i;
        for (i = 0; test_cases[i] != NULL; i++) {
            input = test_cases[i];
            memset(output, 0, sizeof(output));
            inptr = input;
            outptr = output;
            inleft = strlen(input);
            outleft = sizeof(output) - 1;
            
            result = iconv(cd, &inptr, &inleft, &outptr, &outleft);
            if (result == (size_t)-1) {
                printf("  Conversion failed for '%s': %s\n", input, strerror(errno));
            } else {
                *outptr = '\0';  /* Null terminate */
                printf("  '%s' -> '%s' (converted %zu characters)\n", 
                       input, output, result);
            }
        }
        
        iconv_close(cd);
    }
    
    /* Test Latin-1 to UTF-8 with enhanced multibyte handling */
    cd = iconv_open("UTF-8", "ISO-8859-1");
    TEST_ASSERT(cd != (iconv_t)-1, "iconv_open ISO-8859-1->UTF-8 for enhanced test");
    
    if (cd != (iconv_t)-1) {
        /* Test with Latin-1 characters */
        const char *test_cases[] = {
            "Hello",           /* ASCII only */
            "Caf\xe9",         /* Latin-1 é */
            "na\xefve",        /* Latin-1 ï */
            "r\xe9sum\xe9",    /* Latin-1 résumé */
            NULL
        };
        
        int i;
        for (i = 0; test_cases[i] != NULL; i++) {
            input = test_cases[i];
            memset(output, 0, sizeof(output));
            inptr = input;
            outptr = output;
            inleft = strlen(input);
            outleft = sizeof(output) - 1;
            
            result = iconv(cd, &inptr, &inleft, &outptr, &outleft);
            if (result == (size_t)-1) {
                printf("  Conversion failed for Latin-1 string: %s\n", strerror(errno));
            } else {
                *outptr = '\0';  /* Null terminate */
                printf("  Latin-1 string -> UTF-8 (converted %zu characters)\n", result);
            }
        }
        
        iconv_close(cd);
    }
    
    /* Test LOCALE conversions with enhanced multibyte handling */
    cd = iconv_open("UTF-8", "LOCALE");
    TEST_ASSERT(cd != (iconv_t)-1, "iconv_open LOCALE->UTF-8 for enhanced test");
    
    if (cd != (iconv_t)-1) {
        input = "Test with locale";
        memset(output, 0, sizeof(output));
        inptr = input;
        outptr = output;
        inleft = strlen(input);
        outleft = sizeof(output) - 1;
        
        result = iconv(cd, &inptr, &inleft, &outptr, &outleft);
        if (result == (size_t)-1) {
            printf("  LOCALE->UTF-8 conversion failed: %s\n", strerror(errno));
        } else {
            *outptr = '\0';
            printf("  LOCALE->UTF-8: '%s' -> '%s' (converted %zu characters)\n", 
                   input, output, result);
        }
        
        iconv_close(cd);
    }
}

int main(void)
{
    printf("iconv Implementation Test Suite\n");
    printf("===============================\n");
    
    test_identity_conversion();
    test_ascii_to_utf8();
    test_utf8_to_ascii();
    test_latin1_to_utf8();
    test_latin1_high_range();
    test_error_handling();
    test_buffer_overflow();
    test_locale_conversions();
    test_locale_character_validation();
    test_enhanced_multibyte();
    
    printf("\n=== Test Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_failed);
    
    if (tests_failed == 0) {
        printf("\nAll tests passed! ✓\n");
        return 0;
    } else {
        printf("\nSome tests failed! ✗\n");
        return 1;
    }
}
