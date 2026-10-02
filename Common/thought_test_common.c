/**
 * @file thought_test_common.c
 * @brief Common implementations for THOUGHT tests
 *
 * This file provides shared implementations for the THOUGHT test suite.
 */

#include "thought_test_common.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Test fixture functions */
void thought_fixture_setup(thought_fixture_t* fixture) {
    TEST_ASSERT_NOT_NULL(fixture);
    
    /* Initialize fixture to default state */
    fixture->outline_buffer = NULL;
    fixture->current_depth = 0;
    fixture->entry_count = 0;
    fixture->mode = 0;  /* Default to Create mode */
}

void thought_fixture_teardown(thought_fixture_t* fixture) {
    TEST_ASSERT_NOT_NULL(fixture);
    
    /* Clean up allocated memory */
    if (fixture->outline_buffer != NULL) {
        free(fixture->outline_buffer);
        fixture->outline_buffer = NULL;
    }
    
    /* Reset fixture state */
    fixture->current_depth = 0;
    fixture->entry_count = 0;
    fixture->mode = 0;
}

/* Assertion helpers */
void assert_outline_node_valid(const char* node) {
    TEST_ASSERT_NOT_NULL(node);
    
    /* Check for valid indicators at start */
    char first_char = node[0];
    TEST_ASSERT_TRUE(
        first_char == INDICATOR_EXPANDABLE ||
        first_char == INDICATOR_COLLAPSED ||
        first_char == INDICATOR_DOCUMENT ||
        first_char == INDICATOR_CLONE ||
        first_char == INDICATOR_NONE
    );
}

void assert_document_content_valid(const char* content) {
    TEST_ASSERT_NOT_NULL(content);
    
    /* Check content length is within bounds */
    size_t len = strlen(content);
    TEST_ASSERT_TRUE(len <= THOUGHT_MAX_DOCUMENT_LEN);
    
    /* Check for valid characters (basic ASCII) */
    for (size_t i = 0; i < len; i++) {
        char c = content[i];
        TEST_ASSERT_TRUE(c >= 0x20 && c <= 0x7E || c == '\n' || c == '\r' || c == '\t');
    }
}

void assert_headline_length_valid(const char* headline) {
    TEST_ASSERT_NOT_NULL(headline);
    
    /* Check headline length */
    size_t len = strlen(headline);
    TEST_ASSERT_TRUE(len <= THOUGHT_MAX_HEADLINE_LEN);
}

/* Utility functions */
const char* mode_to_string(int mode) {
    switch (mode) {
        case 0: return "Create";
        case 1: return "Review/Revise";
        case 2: return "Document";
        default: return "Unknown";
    }
}