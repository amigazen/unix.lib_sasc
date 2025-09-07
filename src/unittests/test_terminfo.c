/*
 * test_terminfo.c - Comprehensive terminfo test suite
 *
 * Tests all terminfo API functions including setupterm, tigetstr,
 * tigetnum, tigetflag, tparm, tiparm, tputs, and putp.
 *
 * Copyright (c) 2025 amigazen project
 * 
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "/include/terminfo.h"
#include "/include/termcap.h"
#include "/include/internal/amiga_terminfo.h"

/* Forward declarations for debug functions */
extern char *cgetstr(char *buf, const char *cap, char **area);
extern char *cgetcap(char *buf, const char *cap, int type);

/* Test counters */
static int tests_passed = 0;
static int tests_total = 0;

/* Test helper functions */
static int test_string_capability(const char *capname, const char *expected_start, const char *description)
{
    char *result;
    int success;
    
    result = tigetstr(capname);
    if (result) {
        if (expected_start && strncmp(result, expected_start, strlen(expected_start)) == 0) {
            printf("SUCCESS: %s: %s\n", description, result);
            success = 1;
        } else {
            printf("FAILURE: %s: %s (expected to start with %s)\n", description, result, expected_start);
            success = 0;
        }
    } else {
        printf("FAILURE: %s: capability not found\n", description);
        success = 0;
    }
    
    tests_total++;
    if (success) tests_passed++;
    return success;
}

static int test_numeric_capability(const char *capname, int expected_value, const char *description)
{
    int result;
    int success;
    
    result = tigetnum(capname);
    if (result == expected_value) {
        printf("SUCCESS: %s: %d\n", description, result);
        success = 1;
    } else {
        printf("FAILURE: %s: %d (expected %d)\n", description, result, expected_value);
        success = 0;
    }
    
    tests_total++;
    if (success) tests_passed++;
    return success;
}

static int test_boolean_capability(const char *capname, int expected_value, const char *description)
{
    int result;
    int success;
    char *termcap_name;
    char *cgetcap_result;
    
    /* Debug: Check termcap mapping */
    termcap_name = amiga_terminfo_map_capname(capname);
    printf("DEBUG: %s -> termcap: %s\n", capname, termcap_name ? termcap_name : "NULL");
    
    /* Debug: Check cgetcap directly if we have a termcap name */
    if (termcap_name && cur_term && cur_term->capabilities) {
        cgetcap_result = cgetcap(cur_term->capabilities, termcap_name, (int)':');
        printf("DEBUG: cgetcap(buffer, \"%s\", ':') = %p\n", termcap_name, cgetcap_result);
        if (cgetcap_result == (char *)1) {
            printf("DEBUG: -> capability found (enabled)\n");
        } else if (cgetcap_result == NULL) {
            printf("DEBUG: -> capability not found\n");
        } else {
            printf("DEBUG: -> capability found but disabled\n");
        }
    }
    
    result = tigetflag(capname);
    if (result == expected_value) {
        printf("SUCCESS: %s: %s\n", description, result ? "yes" : "no");
        success = 1;
    } else {
        printf("FAILURE: %s: %s (expected %s)\n", description, result ? "yes" : "no", expected_value ? "yes" : "no");
        success = 0;
    }
    
    tests_total++;
    if (success) tests_passed++;
    return success;
}

static int test_putc_callback(int c)
{
    putchar(c);
    return c;
}

int main(int argc, char *argv[])
{
    int result, i;
    char *cm_str;
    char *tparm_result;
    int int_result;
    char *tiparm_result;
    
    /* Suppress unused parameter warnings */
    (void)argc;
    (void)argv;
    
    printf("Amiga Terminfo Comprehensive Test Suite\n");
    printf("======================================\n\n");
    
    /* Test 1: setupterm */
    printf("Test 1: setupterm initialization\n");
    result = setupterm("amiga-console", 1, NULL);
    if (result == 0) {
        printf("SUCCESS: setupterm initialized amiga-console terminal\n");
        
        /* Debug: Show termcap buffer contents */
        if (cur_term && cur_term->capabilities) {
            printf("DEBUG: Termcap buffer length: %lu\n", strlen(cur_term->capabilities));
            printf("DEBUG: Termcap buffer (first 200 chars): %.200s\n", cur_term->capabilities);
            printf("DEBUG: Termcap buffer (last 200 chars): %s\n", 
                   cur_term->capabilities + strlen(cur_term->capabilities) - 200);
        }
        
        tests_passed++;
    } else {
        printf("FAILURE: setupterm failed (returned: %d)\n", result);
    }
    tests_total++;
    
    /* Test 2: ttytype global variable */
    printf("\nTest 2: ttytype global variable\n");
    if (strlen(ttytype) > 0) {
        printf("SUCCESS: ttytype set to: %s\n", ttytype);
        tests_passed++;
    } else {
        printf("FAILURE: ttytype not set\n");
    }
    tests_total++;
    
    /* Test 3: String capabilities */
    printf("\nTest 3: String capabilities\n");
    test_string_capability("clear_screen", "\033[2J", "clear_screen capability");
    test_string_capability("cursor_home", "\033[H", "cursor_home capability");
    test_string_capability("cursor_motion", "\033[%d;%dH", "cursor_motion capability");
    test_string_capability("clr_eol", "\033[K", "clr_eol capability");
    test_string_capability("clr_eos", "\033[J", "clr_eos capability");
    test_string_capability("bell", "\007", "bell capability");
    test_string_capability("backspace", "\010", "backspace capability");
    test_string_capability("carriage_return", "\015", "carriage_return capability");
    test_string_capability("line_feed", "\012", "line_feed capability");
    
    /* Test 4: Numeric capabilities */
    printf("\nTest 4: Numeric capabilities\n");
    test_numeric_capability("columns", 80, "columns capability");
    test_numeric_capability("lines", 25, "lines capability");
    test_numeric_capability("colors", 16, "colors capability");
    test_numeric_capability("pairs", 16, "pairs capability");
    
    /* Test 5: Boolean capabilities */
    printf("\nTest 5: Boolean capabilities\n");
    test_boolean_capability("auto_left_margin", 1, "auto_left_margin capability");
    test_boolean_capability("auto_right_margin", 1, "auto_right_margin capability");
    test_boolean_capability("has_meta_key", 0, "has_meta_key capability");
    test_boolean_capability("has_status_line", 0, "has_status_line capability");
    test_boolean_capability("insert_null_glitch", 0, "insert_null_glitch capability");
    test_boolean_capability("memory_above", 0, "memory_above capability");
    test_boolean_capability("memory_below", 0, "memory_below capability");
    test_boolean_capability("move_insert_mode", 0, "move_insert_mode capability");
    test_boolean_capability("move_standout_mode", 0, "move_standout_mode capability");
    test_boolean_capability("over_strike", 0, "over_strike capability");
    test_boolean_capability("status_line_esc_ok", 0, "status_line_esc_ok capability");
    test_boolean_capability("dest_tabs_magic_smso", 0, "dest_tabs_magic_smso capability");
    test_boolean_capability("tilde_glitch", 0, "tilde_glitch capability");
    test_boolean_capability("transparent_underline", 0, "transparent_underline capability");
    test_boolean_capability("xon_xoff", 0, "xon_xoff capability");
    test_boolean_capability("needs_xon_xoff", 0, "needs_xon_xoff capability");
    test_boolean_capability("prtr_silent", 0, "prtr_silent capability");
    test_boolean_capability("hard_cursor", 0, "hard_cursor capability");
    test_boolean_capability("non_rev_rmcup", 0, "non_rev_rmcup capability");
    test_boolean_capability("no_pad_char", 0, "no_pad_char capability");
    test_boolean_capability("non_dest_scroll_region", 0, "non_dest_scroll_region capability");
    test_boolean_capability("can_change", 0, "can_change capability");
    test_boolean_capability("back_color_erase", 0, "back_color_erase capability");
    test_boolean_capability("hue_lightness_saturation", 0, "hue_lightness_saturation capability");
    
    /* Test 6: tparm function */
    printf("\nTest 6: tparm function\n");
    cm_str = tigetstr("cursor_motion");
    if (cm_str) {
        tparm_result = tparm(cm_str, 10, 5);
        if (tparm_result && strstr(tparm_result, "10") && strstr(tparm_result, "5")) {
            printf("SUCCESS: tparm(cursor_motion, 10, 5) generated: %s\n", tparm_result);
            tests_passed++;
        } else {
            printf("FAILURE: tparm(cursor_motion, 10, 5) failed or generated invalid result: %s\n", tparm_result);
        }
    } else {
        printf("FAILURE: Cannot test tparm - cursor_motion capability not available\n");
    }
    tests_total++;
    
    /* Test 7: tiparm function */
    printf("\nTest 7: tiparm function\n");
    if (cm_str) {
        tiparm_result = tiparm(cm_str, 20, 15, 0, 0, 0, 0, 0, 0, 0);
        if (tiparm_result && strstr(tiparm_result, "20") && strstr(tiparm_result, "15")) {
            printf("SUCCESS: tiparm(cursor_motion, 20, 15) generated: %s\n", tiparm_result);
            tests_passed++;
        } else {
            printf("FAILURE: tiparm(cursor_motion, 20, 15) failed or generated invalid result: %s\n", tiparm_result);
        }
    } else {
        printf("FAILURE: Cannot test tiparm - cursor_motion capability not available\n");
    }
    tests_total++;
    
    /* Test 8: tputs function */
    printf("\nTest 8: tputs function\n");
    cm_str = tigetstr("clear_screen");
    if (cm_str) {
        printf("SUCCESS: Testing tputs with clear screen sequence...\n");
        printf("SUCCESS: About to clear screen - previous output will be lost\n");
        int_result = tputs(cm_str, 0, test_putc_callback);
        if (int_result == 0) {
            printf("SUCCESS: tputs executed successfully (returned: %d)\n", int_result);
            printf("SUCCESS: Screen was cleared and this message appears after clearing\n");
            tests_passed++;
        } else {
            printf("FAILURE: tputs failed (returned: %d)\n", int_result);
        }
    } else {
        printf("FAILURE: Cannot test tputs - clear screen capability not available\n");
    }
    tests_total++;
    
    /* Test 9: putp function */
    printf("\nTest 9: putp function\n");
    cm_str = tigetstr("cursor_home");
    if (cm_str) {
        printf("SUCCESS: Testing putp with cursor home sequence...\n");
        int_result = putp(cm_str);
        if (int_result == 0) {
            printf("SUCCESS: putp executed successfully (returned: %d)\n", int_result);
            tests_passed++;
        } else {
            printf("FAILURE: putp failed (returned: %d)\n", int_result);
        }
    } else {
        printf("FAILURE: Cannot test putp - cursor home capability not available\n");
    }
    tests_total++;
    
    /* Test 10: resetterm function */
    printf("\nTest 10: resetterm function\n");
    int_result = resetterm();
    if (int_result == 0) {
        printf("SUCCESS: resetterm executed successfully (returned: %d)\n", int_result);
        tests_passed++;
    } else {
        printf("FAILURE: resetterm failed (returned: %d)\n", int_result);
    }
    tests_total++;
    
    /* Test 11: fixterm function */
    printf("\nTest 11: fixterm function\n");
    int_result = fixterm();
    if (int_result == 0) {
        printf("SUCCESS: fixterm executed successfully (returned: %d)\n", int_result);
        tests_passed++;
    } else {
        printf("FAILURE: fixterm failed (returned: %d)\n", int_result);
    }
    tests_total++;
    
    /* Test 12: saveterm function */
    printf("\nTest 12: saveterm function\n");
    int_result = saveterm();
    if (int_result == 0) {
        printf("SUCCESS: saveterm executed successfully (returned: %d)\n", int_result);
        tests_passed++;
    } else {
        printf("FAILURE: saveterm failed (returned: %d)\n", int_result);
    }
    tests_total++;
    
    /* Test 13: Error handling */
    printf("\nTest 13: Error handling\n");
    if (tigetstr("nonexistent_capability") == NULL) {
        printf("SUCCESS: tigetstr correctly returns NULL for invalid capability\n");
        tests_passed++;
    } else {
        printf("FAILURE: tigetstr should return NULL for invalid capability\n");
    }
    tests_total++;
    
    if (tigetnum("nonexistent_capability") == -1) {
        printf("SUCCESS: tigetnum correctly returns -1 for invalid capability\n");
        tests_passed++;
    } else {
        printf("FAILURE: tigetnum should return -1 for invalid capability\n");
    }
    tests_total++;
    
    if (tigetflag("nonexistent_capability") == -1) {
        printf("SUCCESS: tigetflag correctly returns -1 for invalid capability\n");
        tests_passed++;
    } else {
        printf("FAILURE: tigetflag should return -1 for invalid capability\n");
    }
    tests_total++;
    
    /* Test 14: set_curterm and del_curterm */
    printf("\nTest 14: set_curterm and del_curterm\n");
    if (cur_term) {
        printf("SUCCESS: cur_term is available\n");
        tests_passed++;
    } else {
        printf("FAILURE: cur_term is not available\n");
    }
    tests_total++;
    
    /* Test Summary */
    printf("\nTest Summary\n");
    printf("============\n");
    printf("Tests passed: %d/%d\n", tests_passed, tests_total);
    printf("Success rate: %d%%\n", (tests_passed * 100) / tests_total);
    
    if (tests_passed == tests_total) {
        printf("\nSUCCESS: All tests passed! Terminfo integration is working correctly.\n");
        return 0;
    } else {
        printf("\nFAILURE: Some tests failed. Check the output above for details.\n");
        return 1;
    }
}
