/**
 * @file thought_test_common.h
 * @brief Common definitions and utilities for THOUGHT tests
 *
 * This header provides shared definitions for the THOUGHT test suite.
 * It includes Unity testing framework integration and common constants.
 */

#ifndef THOUGHT_TEST_COMMON_H
#define THOUGHT_TEST_COMMON_H

/* Unity framework includes */
#include "unity.h"

/* Common THOUGHT constants */
#define THOUGHT_MAX_HEADLINE_LEN     240
#define THOUGHT_MAX_DOCUMENT_LEN     4096
#define THOUGHT_MAX_OUTLINE_ENTRIES  256
#define THOUGHT_MAX_DEPTH            32
#define THOUGHT_FILENAME_LEN         12

/* File extensions */
#define THOUGHT_EXTENSION ".CT"
#define DOCUMENT_EXTENSION ".DO"
#define TEXT_EXTENSION ".TX"

/* Visual indicators */
#define INDICATOR_EXPANDABLE '+'   /* Heading with hidden children */
#define INDICATOR_COLLAPSED '-'    /* Heading with visible children */
#define INDICATOR_DOCUMENT '.'     /* Document reference */
#define INDICATOR_CLONE '&'        /* Cloned heading */
#define INDICATOR_CREATE '?'       /* Create mode indicator */
#define INDICATOR_NONE '\0'        /* Leaf node */

/* Function key codes */
#define F1_KEY 0x81
#define F2_KEY 0x82
#define F3_KEY 0x83
#define F4_KEY 0x84
#define F5_KEY 0x85
#define F6_KEY 0x86
#define F7_KEY 0x87
#define F8_KEY 0x88

/* Special key combinations */
#define CTRL_DEL_BKSP 0x0800
#define SHIFT_F2 0x9002
#define SHIFT_F3 0x9003

/* Error codes */
#define THOUGHT_SUCCESS 0
#define THOUGHT_ERR_INVALID_MODE -1
#define THOUGHT_ERR_INVALID_ENTRY -2
#define THOUGHT_ERR_OUTLINE_FULL -3
#define THOUGHT_ERR_OUT_OF_MEMORY -4
#define THOUGHT_ERR_FILE_NOT_FOUND -5
#define THOUGHT_ERR_IO_ERROR -6

/* Test macros */
#define TEST_ASSERT_OUTLINE_EQ(expected, actual) \
    TEST_ASSERT_EQUAL_STRING(expected, actual)

#define TEST_ASSERT_DOCUMENT_EQ(expected, actual) \
    TEST_ASSERT_EQUAL_STRING(expected, actual)

#define TEST_ASSERT_ERROR(expected, actual) \
    TEST_ASSERT_EQUAL_INT(expected, actual)

/* Test fixture for outline state */
typedef struct {
    char* outline_buffer;
    int current_depth;
    int entry_count;
    int mode;  /* 0 = Create, 1 = Review/Revise, 2 = Document */
} thought_fixture_t;

/* Test fixture functions */
void thought_fixture_setup(thought_fixture_t* fixture);
void thought_fixture_teardown(thought_fixture_t* fixture);

/* Common assertion helpers */
void assert_outline_node_valid(const char* node);
void assert_document_content_valid(const char* content);
void assert_headline_length_valid(const char* headline);

#endif /* THOUGHT_TEST_COMMON_H */