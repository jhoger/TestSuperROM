/**
 * @file test_thought_driver.c
 * @brief Main test driver for THOUGHT tests
 *
 * This is the main entry point for all THOUGHT unit tests.
 * It initializes the Unity test framework and runs all registered tests.
 */

/* Unity framework */
#include "unity.h"
#include "vt_interact.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#endif

/* Test includes - add new test files here */
#include "test_thought_outline.h"
#include "test_thought_navigation.h"
#include "test_thought_modes.h"
#include "test_thought_hierarchy.h"
#include "test_thought_documents.h"
#include "test_thought_drag.h"
#include "test_thought_clone.h"
#include "test_thought_file.h"
#include "test_thought_create.h"
#include "test_thought_view.h"
#include "test_thought_deletion.h"
#include "thought_test_common.h"

/* External variables from Unity - used for command line filtering */
#ifdef UNITY_USE_COMMAND_LINE_ARGS
extern char* UnityOptionIncludeNamed;
extern char* UnityOptionExcludeNamed;
#endif

/* Test function declarations */

/* Outline module tests */
void test_outline_creation(void);
void test_outline_deletion(void);
void test_outline_expansion(void);
void test_outline_collapse(void);

/* Navigation module tests */
void test_navigation_cursor_movement(void);
void test_navigation_view_management(void);

/* Modes module tests */
void test_thought_mode_create_enter(void);
void test_thought_mode_create_to_review(void);
void test_thought_mode_visual_indicators(void);

/* Hierarchy module tests */
void test_thought_hierarchy_parent_child(void);
void test_thought_hierarchy_deep_nesting(void);
void test_thought_hierarchy_siblings(void);

/* Documents module tests */
void test_thought_document_reference(void);
void test_thought_clone_reference(void);
void test_thought_multiple_documents(void);

/* Drag module tests */
void test_thought_drag_heading(void);
void test_thought_drag_section_with_children(void);
void test_thought_drag_reorder_headings(void);

/* Clone module tests */
void test_thought_clone_heading(void);
void test_thought_clone_section_with_children(void);
void test_thought_clone_and_modify(void);

/* File module tests */
void test_thought_file_save(void);
void test_thought_file_load(void);
void test_thought_file_preserve_structure(void);

/* Create module tests */
void test_thought_create_single_heading(void);
void test_thought_create_multiple_headings(void);
void test_thought_create_nested_headings(void);

/* View module tests */
void test_thought_view_hide_heading(void);
void test_thought_view_show_heading(void);
void test_thought_view_multiple_hide_show(void);

/* Deletion module tests */
void test_thought_delete_heading(void);
void test_thought_delete_section_with_children(void);
void test_thought_delete_multiple_headings(void);

/* Shared handle for all tests */
vt_interact_handle_t handle = NULL;

/* Setup and teardown for Unity */
void setUp(void) {
    handle = NULL;
}

void tearDown(void) {
    if (handle) {
        vt_interact_terminate(handle);
        vt_interact_destroy(handle);
        handle = NULL;
    }
    Sleep(500);
}

/* Helper macro to conditionally run a test based on filter */
#ifdef UNITY_USE_COMMAND_LINE_ARGS
#define RUN_TEST_IF_MATCH(test_func) \
    do { \
        const char* test_name = #test_func; \
        Unity.CurrentTestName = test_name; \
        Unity.CurrentTestLineNumber = __LINE__; \
        if (UnityTestMatches()) { \
            UnityDefaultTestRun(test_func, test_name, __LINE__); \
        } else { \
            Unity.NumberOfTests++; \
            if (UnityOptionIncludeNamed) { \
                UnityPrint("IGNORE "); \
                UnityPrint(test_name); \
                UNITY_OUTPUT_CHAR('\n'); \
            } \
        } \
    } while (0)
#else
#define RUN_TEST_IF_MATCH(test_func) RUN_TEST(test_func)
#endif

int main(int argc, char** argv) {
    /* Parse command line arguments for test filtering */
    UnityParseOptions(argc, argv);

    /* Initialize Unity framework */
    UNITY_BEGIN();

    /* Run all registered tests (only those matching filter if provided) */
    RUN_TEST_IF_MATCH(test_outline_creation);
    RUN_TEST_IF_MATCH(test_outline_deletion);
    RUN_TEST_IF_MATCH(test_outline_expansion);
    RUN_TEST_IF_MATCH(test_outline_collapse);
    RUN_TEST_IF_MATCH(test_navigation_cursor_movement);
    RUN_TEST_IF_MATCH(test_navigation_view_management);
    RUN_TEST_IF_MATCH(test_thought_mode_create_enter);
    RUN_TEST_IF_MATCH(test_thought_mode_create_to_review);
    RUN_TEST_IF_MATCH(test_thought_mode_visual_indicators);
    RUN_TEST_IF_MATCH(test_thought_hierarchy_parent_child);
    RUN_TEST_IF_MATCH(test_thought_hierarchy_deep_nesting);
    RUN_TEST_IF_MATCH(test_thought_hierarchy_siblings);
    RUN_TEST_IF_MATCH(test_thought_document_reference);
    RUN_TEST_IF_MATCH(test_thought_clone_reference);
    RUN_TEST_IF_MATCH(test_thought_multiple_documents);
    RUN_TEST_IF_MATCH(test_thought_drag_heading);
    RUN_TEST_IF_MATCH(test_thought_drag_section_with_children);
    RUN_TEST_IF_MATCH(test_thought_drag_reorder_headings);
    RUN_TEST_IF_MATCH(test_thought_clone_heading);
    RUN_TEST_IF_MATCH(test_thought_clone_section_with_children);
    RUN_TEST_IF_MATCH(test_thought_clone_and_modify);
    RUN_TEST_IF_MATCH(test_thought_file_save);
    RUN_TEST_IF_MATCH(test_thought_file_load);
    RUN_TEST_IF_MATCH(test_thought_file_preserve_structure);
    RUN_TEST_IF_MATCH(test_thought_create_single_heading);
    RUN_TEST_IF_MATCH(test_thought_create_multiple_headings);
    RUN_TEST_IF_MATCH(test_thought_create_nested_headings);
    RUN_TEST_IF_MATCH(test_thought_view_hide_heading);
    RUN_TEST_IF_MATCH(test_thought_view_show_heading);
    RUN_TEST_IF_MATCH(test_thought_view_multiple_hide_show);
    RUN_TEST_IF_MATCH(test_thought_delete_heading);
    RUN_TEST_IF_MATCH(test_thought_delete_section_with_children);
    RUN_TEST_IF_MATCH(test_thought_delete_multiple_headings);

    /* Run all tests */
    return UNITY_END();
}
