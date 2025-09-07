/*
 * Test program for regex.library
 * Tests POSIX regex functionality
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <exec/types.h>
#include <regex.h>
#include <proto/regex.h>
#include <pragmas/regex_pragmas.h>

extern struct RegexBase *RegexBase;

/* Test result tracking */
static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

/* Test macro */
#define TEST(name, condition) do { \
    tests_run++; \
    if (condition) { \
        printf("PASS: %s\n", name); \
        tests_passed++; \
    } else { \
        printf("FAIL: %s\n", name); \
        tests_failed++; \
    } \
} while(0)

/* Test basic regex compilation */
static void test_basic_compilation(void)
{
    regex_t preg;
    int result;
    
    printf("\n=== Testing Basic Compilation ===\n");
    
    /* Test valid pattern */
    result = regcomp(&preg, "hello", 0);
    TEST("Compile simple pattern", result == 0);
    if (result == 0) {
        regfree(&preg);
    }
    
    /* Test invalid pattern */
    result = regcomp(&preg, "[", 0);
    TEST("Compile invalid pattern fails", result != 0);
    
    /* Test extended regex */
    result = regcomp(&preg, "hello.*world", REG_EXTENDED);
    TEST("Compile extended regex", result == 0);
    if (result == 0) {
        regfree(&preg);
    }
    
    /* Test case insensitive */
    result = regcomp(&preg, "HELLO", REG_ICASE);
    TEST("Compile case insensitive", result == 0);
    if (result == 0) {
        regfree(&preg);
    }
}

/* Test regex execution */
static void test_basic_execution(void)
{
    regex_t preg;
    regmatch_t pmatch[10];
    int result;
    
    printf("\n=== Testing Basic Execution ===\n");
    
    /* Test simple match */
    result = regcomp(&preg, "hello", 0);
    if (result == 0) {
        result = regexec(&preg, "hello world", 0, NULL, 0);
        TEST("Simple match", result == 0);
        regfree(&preg);
    }
    
    /* Test no match */
    result = regcomp(&preg, "goodbye", 0);
    if (result == 0) {
        result = regexec(&preg, "hello world", 0, NULL, 0);
        TEST("No match", result == REG_NOMATCH);
        regfree(&preg);
    }
    
    /* Test subexpression capture */
    result = regcomp(&preg, "hello (.*) world", REG_EXTENDED);
    if (result == 0) {
        result = regexec(&preg, "hello beautiful world", 10, pmatch, 0);
        TEST("Subexpression capture", result == 0 && pmatch[1].rm_so == 6 && pmatch[1].rm_eo == 15);
        regfree(&preg);
    }
}

/* Test error handling */
static void test_error_handling(void)
{
    regex_t preg;
    char errbuf[256];
    size_t len;
    
    printf("\n=== Testing Error Handling ===\n");
    
    /* Test error message for invalid pattern */
    regcomp(&preg, "[", 0);
    len = regerror(REG_EBRACK, &preg, errbuf, sizeof(errbuf));
    TEST("Error message generation", len > 0 && strlen(errbuf) > 0);
    
    /* Test error message for no match */
    len = regerror(REG_NOMATCH, NULL, errbuf, sizeof(errbuf));
    TEST("No match error message", len > 0 && strlen(errbuf) > 0);
}

/* Test ARexx interface */
static void test_arexx_interface(void)
{
    regmatch_t pmatch;
    LONG result;
    
    printf("\n=== Testing ARexx Interface ===\n");
    
    /* Test basic ARexx match */
    result = rematch("hello", "hello world", 0, &pmatch);
    TEST("ARexx basic match", result == 0);
    
    /* Test ARexx no match */
    result = rematch("goodbye", "hello world", 0, &pmatch);
    TEST("ARexx no match", result != 0);
    
    /* Test case insensitive ARexx match */
    result = rematch("HELLO", "hello world", REGEX_ICASE, &pmatch);
    TEST("ARexx case insensitive match", result == 0);
}

/* Test complex patterns */
static void test_complex_patterns(void)
{
    regex_t preg;
    regmatch_t pmatch[10];
    int result;
    
    printf("\n=== Testing Complex Patterns ===\n");
    
    /* Test character classes */
    result = regcomp(&preg, "[0-9]+", REG_EXTENDED);
    if (result == 0) {
        result = regexec(&preg, "abc123def", 10, pmatch, 0);
        TEST("Character class match", result == 0 && pmatch[0].rm_so == 3 && pmatch[0].rm_eo == 6);
        regfree(&preg);
    }
    
    /* Test quantifiers */
    result = regcomp(&preg, "a+", REG_EXTENDED);
    if (result == 0) {
        result = regexec(&preg, "aaa", 10, pmatch, 0);
        TEST("Quantifier match", result == 0 && pmatch[0].rm_so == 0 && pmatch[0].rm_eo == 3);
        regfree(&preg);
    }
    
    /* Test anchors */
    result = regcomp(&preg, "^hello", REG_EXTENDED);
    if (result == 0) {
        result = regexec(&preg, "hello world", 10, pmatch, 0);
        TEST("Anchor match", result == 0);
        regfree(&preg);
    }
}

/* Main test function */
int main(void)
{

    RegexBase = (struct RegexBase *)OpenLibrary("progdir:/regex.library", 0);
    if (!RegexBase) {
        printf("Failed to open regex.library\n");
        return 1;
    }

    printf("Regex Library Test Suite\n");
    printf("=======================\n");
    
    test_basic_compilation();
    test_basic_execution();
    test_error_handling();
    test_arexx_interface();
    test_complex_patterns();
    
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

    CloseLibrary(RegexBase);
}
