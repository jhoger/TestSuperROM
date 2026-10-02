/**
 * @file test_thought_driver.c
 * @brief Main test driver for THOUGHT tests
 *
 * This is the main entry point for all THOUGHT unit tests.
 * It initializes the Unity test framework and runs all registered tests.
 */

/* Unity framework */
#include "unity.h"

/* Test includes - add new test files here */
#include "test_thought_outline.h"
#include "test_thought_navigation.h"
#include "test_thought_modes.h"
#include "test_thought_file_io.h"
#include "test_thought_print.h"

/* Test function declarations */
void test_outline_creation(void);
void test_outline_deletion(void);
void test_outline_expansion(void);
void test_outline_collapse(void);
void test_navigation_cursor_movement(void);
void test_navigation_view_management(void);
void test_mode_create_to_review(void);
void test_mode_review_to_document(void);
void test_mode_document_to_review(void);
void test_file_save_load(void);
void test_file_io_errors(void);
void test_print_format_options(void);
void test_print_defaults(void);

/* Test suite registration */
void setUp(void) {
    /* Setup before each test */
}

void tearDown(void) {
    /* Teardown after each test */
}

int main(int argc, char** argv) {
    /* Initialize Unity framework */
    UnityBegin("test_thought_driver.c");
    
    /* Register tests - outline module */
    UnityAddTest(test_outline_creation, "test_outline_creation", TEST_LINE_NUM);
    UnityAddTest(test_outline_deletion, "test_outline_deletion", TEST_LINE_NUM);
    UnityAddTest(test_outline_expansion, "test_outline_expansion", TEST_LINE_NUM);
    UnityAddTest(test_outline_collapse, "test_outline_collapse", TEST_LINE_NUM);
    
    /* Register tests - navigation module */
    UnityAddTest(test_navigation_cursor_movement, "test_navigation_cursor_movement", TEST_LINE_NUM);
    UnityAddTest(test_navigation_view_management, "test_navigation_view_management", TEST_LINE_NUM);
    
    /* Register tests - modes module */
    UnityAddTest(test_mode_create_to_review, "test_mode_create_to_review", TEST_LINE_NUM);
    UnityAddTest(test_mode_review_to_document, "test_mode_review_to_document", TEST_LINE_NUM);
    UnityAddTest(test_mode_document_to_review, "test_mode_document_to_review", TEST_LINE_NUM);
    
    /* Register tests - file I/O module */
    UnityAddTest(test_file_save_load, "test_file_save_load", TEST_LINE_NUM);
    UnityAddTest(test_file_io_errors, "test_file_io_errors", TEST_LINE_NUM);
    
    /* Register tests - print module */
    UnityAddTest(test_print_format_options, "test_print_format_options", TEST_LINE_NUM);
    UnityAddTest(test_print_defaults, "test_print_defaults", TEST_LINE_NUM);
    
    /* Run all tests */
    UnityRun();
    
    /* Return summary */
    return UnityEnd();
}