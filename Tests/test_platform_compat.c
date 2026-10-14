/*
 * Platform Compatibility Layer - Test Suite
 *
 * Comprehensive tests for platform abstraction functions.
 * All tests should initially fail, then pass as implementations are added.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

/* Unity framework */
#include "unity.h"

/* Platform compatibility declarations */
#include "platform_compat.h"

/* ============================================================================
 * STRING COMPATIBILITY TESTS
 * ============================================================================ */

void test_platform_strncpy_s_null_dest_returns_error(void) {
    int result = platform_strncpy_s(NULL, 10, "test", 4);
    TEST_ASSERT_EQUAL_INT(-1, result);
}

void test_platform_strncpy_s_null_src_returns_error(void) {
    char dest[10];
    int result = platform_strncpy_s(dest, 10, NULL, 4);
    TEST_ASSERT_EQUAL_INT(-1, result);
}

void test_platform_strncpy_s_zero_size_returns_error(void) {
    char dest[10];
    int result = platform_strncpy_s(dest, 0, "test", 4);
    TEST_ASSERT_EQUAL_INT(-1, result);
}

void test_platform_strncpy_s_basic_copy(void) {
    char dest[20];
    int result = platform_strncpy_s(dest, 20, "Hello", 5);
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_STRING("Hello", dest);
}

void test_platform_strncpy_s_truncates_at_count(void) {
    char dest[20];
    int result = platform_strncpy_s(dest, 20, "Hello World", 5);
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_STRING("Hello", dest);
}

void test_platform_strncpy_s_truncates_at_dest_size(void) {
    char dest[10];
    int result = platform_strncpy_s(dest, 10, "Hello World", 11);
    TEST_ASSERT_EQUAL_INT(0, result);
    /* dest_size=10, count=11, so we copy min(10, 11-1)=9 chars + null terminator */
    TEST_ASSERT_EQUAL_STRING("Hello Wor", dest);
}

void test_platform_strncpy_s_always_null_terminates(void) {
    char dest[5];
    int result = platform_strncpy_s(dest, 5, "Hello World", 100);
    TEST_ASSERT_EQUAL_INT(0, result);
    /* dest_size=5, count=100, so we copy min(4, 100)=4 chars + null terminator */
    TEST_ASSERT_EQUAL_STRING("Hell", dest);
    TEST_ASSERT_EQUAL('\0', dest[4]);
}

void test_platform_strcpy_s_null_dest_returns_error(void) {
    int result = platform_strcpy_s(NULL, 10, "test");
    TEST_ASSERT_EQUAL_INT(-1, result);
}

void test_platform_strcpy_s_null_src_returns_error(void) {
    char dest[10];
    int result = platform_strcpy_s(dest, 10, NULL);
    TEST_ASSERT_EQUAL_INT(-1, result);
}

void test_platform_strcpy_s_zero_size_returns_error(void) {
    char dest[10];
    int result = platform_strcpy_s(dest, 0, "test");
    TEST_ASSERT_EQUAL_INT(-1, result);
}

void test_platform_strcpy_s_basic_copy(void) {
    char dest[20];
    int result = platform_strcpy_s(dest, 20, "Hello World");
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_STRING("Hello World", dest);
}

void test_platform_strcpy_s_empty_string(void) {
    char dest[20];
    int result = platform_strcpy_s(dest, 20, "");
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_STRING("", dest);
}

void test_platform_strncat_s_null_dest_returns_error(void) {
    int result = platform_strncat_s(NULL, 10, "test", 4);
    TEST_ASSERT_EQUAL_INT(-1, result);
}

void test_platform_strncat_s_null_src_returns_error(void) {
    char dest[10] = "Hello";
    int result = platform_strncat_s(dest, 10, NULL, 4);
    TEST_ASSERT_EQUAL_INT(-1, result);
}

void test_platform_strncat_s_zero_size_returns_error(void) {
    char dest[10] = "Hello";
    int result = platform_strncat_s(dest, 0, "test", 4);
    TEST_ASSERT_EQUAL_INT(-1, result);
}

void test_platform_strncat_s_basic_concat(void) {
    char dest[20] = "Hello";
    int result = platform_strncat_s(dest, 20, " World", 7);
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_STRING("Hello World", dest);
}

void test_platform_strncat_s_truncates_at_count(void) {
    char dest[20] = "Hello";
    /* count=5, dest has 5 chars, remaining space is 14, so we append min(5,14)=5 chars */
    /* strncat appends 5 chars from " World!" = " Worl" */
    int result = platform_strncat_s(dest, 20, " World!", 5);
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_STRING("Hello Worl", dest);
}

void test_platform_strncat_s_truncates_at_dest_size(void) {
    char dest[15] = "Hello";
    /* dest has 5 chars, remaining space is 9, so we append min(8,9)=8 chars */
    /* strncat appends 8 chars from " World!" = " World!" */
    int result = platform_strncat_s(dest, 15, " World!", 8);
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_STRING("Hello World!", dest);
}

void test_platform_strncat_s_empty_source(void) {
    char dest[20] = "Hello";
    int result = platform_strncat_s(dest, 20, "", 0);
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_STRING("Hello", dest);
}

void test_platform_strncat_s_dest_full_returns_error(void) {
    char dest[10] = "Hello World";
    int result = platform_strncat_s(dest, 10, " test", 5);
    TEST_ASSERT_EQUAL_INT(-1, result);
}

void test_platform_strdup_null_src_returns_null(void) {
    char *result = platform_strdup(NULL);
    TEST_ASSERT_NULL(result);
}

void test_platform_strdup_basic_copy(void) {
    char *result = platform_strdup("Hello World");
    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_STRING("Hello World", result);
    free(result);
}

void test_platform_strdup_empty_string(void) {
    char *result = platform_strdup("");
    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_STRING("", result);
    TEST_ASSERT_EQUAL(0, strlen(result));  /* Empty string has length 0 */
    free(result);
}

void test_platform_strdup_duplicate_content(void) {
    const char *original = "Test String";
    char *result = platform_strdup(original);
    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_STRING(original, result);

    /* Modify original, verify duplicate is independent */
    char *mutable = malloc(strlen(original) + 1);
    strcpy(mutable, original);
    strcpy(mutable, "Modified");
    TEST_ASSERT_EQUAL_STRING(original, result);

    free(mutable);
    free(result);
}

/* ============================================================================
 * FILE COMPATIBILITY TESTS
 * ============================================================================ */

void test_platform_getcwd_success(void) {
    char buf[256];
    int result = platform_getcwd(buf, sizeof(buf));
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_NOT_NULL(buf);
    TEST_ASSERT_GREATER_THAN(0, strlen(buf));
}

void test_platform_getcwd_buffer_too_small(void) {
    char buf[1];
    int result = platform_getcwd(buf, sizeof(buf));
    TEST_ASSERT_EQUAL_INT(-1, result);
}

void test_platform_getcwd_empty_buffer(void) {
    char buf[0];
    int result = platform_getcwd(buf, sizeof(buf));
    TEST_ASSERT_EQUAL_INT(-1, result);
}

void test_platform_realpath_basic(void) {
    char resolved[512];
    char *result = platform_realpath(".", resolved);
    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_NOT_NULL(resolved);
    TEST_ASSERT_GREATER_THAN(0, strlen(resolved));
    free(result);
}

void test_platform_realpath_null_path_returns_null(void) {
    char resolved[512];
    char *result = platform_realpath(NULL, resolved);
    TEST_ASSERT_NULL(result);
}

void test_platform_realpath_nonexistent_file(void) {
    char resolved[512];
    char *result = platform_realpath("/nonexistent/path/file.txt", resolved);
    TEST_ASSERT_NULL(result);
}

void test_platform_realpath_relative_path_converted(void) {
    char resolved[512];
    char *result = platform_realpath("../Common", resolved);
    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_NOT_NULL(resolved);
    /* Should be an absolute path */
    TEST_ASSERT_EQUAL_CHAR('/', resolved[0]);
    free(result);
}

/* ============================================================================
 * SLEEP TESTS
 * ============================================================================ */

void test_platform_sleep_ms_basic(void) {
    struct timeval start, end;
    uint32_t duration_ms = 100;

    gettimeofday(&start, NULL);
    platform_sleep_ms(duration_ms);
    gettimeofday(&end, NULL);

    long elapsed_ms = (end.tv_sec - start.tv_sec) * 1000 +
                      (end.tv_usec - start.tv_usec) / 1000;

    TEST_ASSERT_GREATER_OR_EQUAL_INT(duration_ms - 50, elapsed_ms);
    TEST_ASSERT_LESS_THAN_INT(duration_ms + 50, elapsed_ms);
}

void test_platform_sleep_ms_zero(void) {
    struct timeval start, end;

    gettimeofday(&start, NULL);
    platform_sleep_ms(0);
    gettimeofday(&end, NULL);

    /* Should return immediately */
    long elapsed_ms = (end.tv_sec - start.tv_sec) * 1000 +
                      (end.tv_usec - start.tv_usec) / 1000;
    TEST_ASSERT_LESS_THAN_INT(50, elapsed_ms);
}

void test_platform_sleep_ms_small_duration(void) {
    struct timeval start, end;
    uint32_t duration_ms = 10;

    gettimeofday(&start, NULL);
    platform_sleep_ms(duration_ms);
    gettimeofday(&end, NULL);

    long elapsed_ms = (end.tv_sec - start.tv_sec) * 1000 +
                      (end.tv_usec - start.tv_usec) / 1000;

    TEST_ASSERT_LESS_THAN_INT(duration_ms + 50, elapsed_ms);
}

void test_platform_sleep_us_basic(void) {
    struct timeval start, end;
    uint32_t duration_us = 100000;  /* 100ms */

    gettimeofday(&start, NULL);
    platform_sleep_us(duration_us);
    gettimeofday(&end, NULL);

    long elapsed_us = (end.tv_sec - start.tv_sec) * 1000000 +
                      (end.tv_usec - start.tv_usec);

    TEST_ASSERT_GREATER_OR_EQUAL_INT(duration_us - 50000, elapsed_us);
    TEST_ASSERT_LESS_THAN_INT(duration_us + 50000, elapsed_us);
}

void test_platform_sleep_us_zero(void) {
    struct timeval start, end;

    gettimeofday(&start, NULL);
    platform_sleep_us(0);
    gettimeofday(&end, NULL);

    /* Should return immediately */
    long elapsed_us = (end.tv_sec - start.tv_sec) * 1000000 +
                      (end.tv_usec - start.tv_usec);
    TEST_ASSERT_LESS_THAN_INT(50000, elapsed_us);
}

void test_platform_sleep_us_small_duration(void) {
    struct timeval start, end;
    uint32_t duration_us = 1000;  /* 1ms */

    gettimeofday(&start, NULL);
    platform_sleep_us(duration_us);
    gettimeofday(&end, NULL);

    long elapsed_us = (end.tv_sec - start.tv_sec) * 1000000 +
                      (end.tv_usec - start.tv_usec);

    TEST_ASSERT_LESS_THAN_INT(duration_us + 50000, elapsed_us);
}

/* ============================================================================
 * THREAD SYNCHRONIZATION TESTS
 * ============================================================================ */

static int g_test_counter;
static platform_mutex_t g_test_mutex;

static void* test_thread_func(void* arg) {
    (void)arg;
    platform_mutex_lock(&g_test_mutex);
    g_test_counter++;
    platform_mutex_unlock(&g_test_mutex);
    return NULL;
}

void test_platform_mutex_init_success(void) {
    platform_mutex_t mutex;
    int result = platform_mutex_init(&mutex);
    TEST_ASSERT_EQUAL_INT(0, result);
    platform_mutex_destroy(&mutex);
}

void test_platform_mutex_init_null_mutex_returns_error(void) {
    int result = platform_mutex_init(NULL);
    TEST_ASSERT_EQUAL_INT(-1, result);
}

void test_platform_mutex_lock_unlock_basic(void) {
    platform_mutex_t mutex;
    platform_mutex_init(&mutex);

    platform_mutex_lock(&mutex);
    platform_mutex_unlock(&mutex);

    platform_mutex_destroy(&mutex);
}

void test_platform_mutex_lock_unlock_multiple(void) {
    platform_mutex_t mutex;
    platform_mutex_init(&mutex);

    for (int i = 0; i < 100; i++) {
        platform_mutex_lock(&mutex);
        platform_mutex_unlock(&mutex);
    }

    platform_mutex_destroy(&mutex);
}

void test_platform_mutex_lock_unlock_race_condition(void) {
    platform_mutex_t mutex;
    platform_mutex_init(&mutex);
    g_test_counter = 0;

    /* Create multiple threads that increment counter */
    pthread_t threads[10];
    for (int i = 0; i < 10; i++) {
        pthread_create(&threads[i], NULL, test_thread_func, NULL);
    }

    /* Wait for all threads to complete */
    for (int i = 0; i < 10; i++) {
        pthread_join(threads[i], NULL);
    }

    platform_mutex_destroy(&mutex);

    /* All increments should be accounted for */
    TEST_ASSERT_EQUAL_INT(10, g_test_counter);
}

void test_platform_mutex_destroy_after_use(void) {
    platform_mutex_t mutex;
    platform_mutex_init(&mutex);

    platform_mutex_lock(&mutex);
    platform_mutex_unlock(&mutex);

    /* Destroy should succeed */
    platform_mutex_destroy(&mutex);

    /* Destroy again should be safe or at least not crash */
    platform_mutex_destroy(&mutex);
}

/* ============================================================================
 * ERROR HANDLING TESTS
 * ============================================================================ */

void test_platform_get_last_error_after_success_returns_zero(void) {
    /* Clear any previous error first */
    errno = 0;
    int err = platform_get_last_error();
    /* After a successful operation, error should be 0 */
    TEST_ASSERT_EQUAL_INT(0, err);
}

void test_platform_get_last_error_after_failure(void) {
    /* Create an error condition */
    FILE *f = fopen("/nonexistent/file.txt", "r");
    if (f == NULL) {
        int err = platform_get_last_error();
        TEST_ASSERT_GREATER_THAN(0, err);
    }
}

void test_platform_strerror_valid_error(void) {
    /* Set an error */
    FILE *f = fopen("/nonexistent/file.txt", "r");
    int err = 0;
    if (f == NULL) {
        err = platform_get_last_error();
    }

    const char *msg = platform_strerror(err);
    TEST_ASSERT_NOT_NULL(msg);
    TEST_ASSERT_GREATER_THAN(0, strlen(msg));
}

void test_platform_strerror_zero_returns_ok_message(void) {
    const char *msg = platform_strerror(0);
    TEST_ASSERT_NOT_NULL(msg);
    TEST_ASSERT_GREATER_THAN(0, strlen(msg));
}

void test_platform_strerror_invalid_error_returns_message(void) {
    const char *msg = platform_strerror(99999);
    TEST_ASSERT_NOT_NULL(msg);
    TEST_ASSERT_GREATER_THAN(0, strlen(msg));
}

/* ============================================================================
 * STRING COMPARISON TESTS (case-insensitive)
 * ============================================================================ */

void test_platform_stricmp_equal_strings(void) {
    int result = platform_stricmp("Hello", "Hello");
    TEST_ASSERT_EQUAL_INT(0, result);
}

void test_platform_stricmp_case_insensitive_equal(void) {
    int result = platform_stricmp("Hello", "HELLO");
    TEST_ASSERT_EQUAL_INT(0, result);
}

void test_platform_stricmp_case_insensitive_equal_mixed(void) {
    int result = platform_stricmp("HeLLo WoRLd", "hElLo wOrLd");
    TEST_ASSERT_EQUAL_INT(0, result);
}

void test_platform_stricmp_not_equal(void) {
    int result = platform_stricmp("Hello", "World");
    TEST_ASSERT_NOT_EQUAL_INT(0, result);
}

void test_platform_stricmp_different_case_not_equal(void) {
    int result = platform_stricmp("Hello", "hello");
    /* Case-insensitive comparison should be equal */
    TEST_ASSERT_EQUAL_INT(0, result);
}

void test_platform_stricmp_null_first_returns_nonzero(void) {
    int result = platform_stricmp(NULL, "Hello");
    TEST_ASSERT_NOT_EQUAL_INT(0, result);
}

void test_platform_stricmp_null_second_returns_nonzero(void) {
    int result = platform_stricmp("Hello", NULL);
    TEST_ASSERT_NOT_EQUAL_INT(0, result);
}

void test_platform_stricmp_empty_strings(void) {
    int result = platform_stricmp("", "");
    TEST_ASSERT_EQUAL_INT(0, result);
}

void test_platform_stricmp_empty_vs_nonempty(void) {
    int result = platform_stricmp("", "Hello");
    TEST_ASSERT_NOT_EQUAL_INT(0, result);
}

void test_platform_stricmp_numeric_strings(void) {
    int result = platform_stricmp("123", "123");
    TEST_ASSERT_EQUAL_INT(0, result);
}

/* ============================================================================
 * EDGE CASE TESTS
 * ============================================================================ */

void test_platform_strncpy_s_exact_size(void) {
    char dest[6];
    int result = platform_strncpy_s(dest, 6, "Hello", 5);
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_STRING("Hello", dest);
}

void test_platform_strncpy_s_one_byte_over_dest_size(void) {
    char dest[5];
    int result = platform_strncpy_s(dest, 5, "Hello", 5);
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_STRING("Hell", dest);
}

void test_platform_strncat_s_dest_with_null_terminator(void) {
    char dest[20] = "Hello";
    int result = platform_strncat_s(dest, 20, "", 0);
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_STRING("Hello", dest);
}

void test_platform_strdup_very_long_string(void) {
    char long_str[10000];
    memset(long_str, 'A', sizeof(long_str) - 1);
    long_str[sizeof(long_str) - 1] = '\0';

    char *result = platform_strdup(long_str);
    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_INT(strlen(long_str), strlen(result));
    TEST_ASSERT_EQUAL_STRING(long_str, result);

    free(result);
}

void test_platform_getcwd_absolute_path(void) {
    char buf[256];
    platform_getcwd(buf, sizeof(buf));

    /* Should start with / on Linux */
    TEST_ASSERT_EQUAL_CHAR('/', buf[0]);
}

void test_platform_realpath_symlink_handling(void) {
    /* Test with /etc/localtime which is typically a symlink */
    char resolved[512];
    char *result = platform_realpath("/etc/localtime", resolved);

    /* Should resolve the symlink */
    if (result != NULL) {
        TEST_ASSERT_NOT_NULL(resolved);
        free(result);
    }
}

/* ============================================================================
 * BENCHMARK TESTS (Performance checks)
 * ============================================================================ */

void test_platform_sleep_ms_performance(void) {
    uint32_t duration_ms = 50;
    struct timeval start, end;
    long elapsed_ms;

    gettimeofday(&start, NULL);
    platform_sleep_ms(duration_ms);
    gettimeofday(&end, NULL);

    elapsed_ms = (end.tv_sec - start.tv_sec) * 1000 +
                 (end.tv_usec - start.tv_usec) / 1000;

    /* Should be reasonably close to requested duration */
    TEST_ASSERT_LESS_THAN_INT(duration_ms * 2, elapsed_ms);
}

void test_platform_mutex_performance(void) {
    platform_mutex_t mutex;
    platform_mutex_init(&mutex);

    struct timeval start, end;
    gettimeofday(&start, NULL);

    for (int i = 0; i < 10000; i++) {
        platform_mutex_lock(&mutex);
        platform_mutex_unlock(&mutex);
    }

    gettimeofday(&end, NULL);
    platform_mutex_destroy(&mutex);

    long elapsed_ms = (end.tv_sec - start.tv_sec) * 1000 +
                      (end.tv_usec - start.tv_usec) / 1000;

    /* 10000 lock/unlock pairs should complete in reasonable time */
    TEST_ASSERT_LESS_THAN_INT(1000, elapsed_ms);
}

/* ============================================================================
 * TEST SUITE SETUP
 * ============================================================================ */

void setUp(void) {
    /* Setup before each test */
}

void tearDown(void) {
    /* Cleanup after each test */
}

int main(void) {
    UNITY_BEGIN();

    /* String compatibility tests */
    RUN_TEST(test_platform_strncpy_s_null_dest_returns_error);
    RUN_TEST(test_platform_strncpy_s_null_src_returns_error);
    RUN_TEST(test_platform_strncpy_s_zero_size_returns_error);
    RUN_TEST(test_platform_strncpy_s_basic_copy);
    RUN_TEST(test_platform_strncpy_s_truncates_at_count);
    RUN_TEST(test_platform_strncpy_s_truncates_at_dest_size);
    RUN_TEST(test_platform_strncpy_s_always_null_terminates);
    RUN_TEST(test_platform_strcpy_s_null_dest_returns_error);
    RUN_TEST(test_platform_strcpy_s_null_src_returns_error);
    RUN_TEST(test_platform_strcpy_s_zero_size_returns_error);
    RUN_TEST(test_platform_strcpy_s_basic_copy);
    RUN_TEST(test_platform_strcpy_s_empty_string);
    RUN_TEST(test_platform_strncat_s_null_dest_returns_error);
    RUN_TEST(test_platform_strncat_s_null_src_returns_error);
    RUN_TEST(test_platform_strncat_s_zero_size_returns_error);
    RUN_TEST(test_platform_strncat_s_basic_concat);
    RUN_TEST(test_platform_strncat_s_truncates_at_count);
    RUN_TEST(test_platform_strncat_s_truncates_at_dest_size);
    RUN_TEST(test_platform_strncat_s_empty_source);
    RUN_TEST(test_platform_strncat_s_dest_full_returns_error);
    RUN_TEST(test_platform_strdup_null_src_returns_null);
    RUN_TEST(test_platform_strdup_basic_copy);
    RUN_TEST(test_platform_strdup_empty_string);
    RUN_TEST(test_platform_strdup_duplicate_content);

    /* File compatibility tests */
    RUN_TEST(test_platform_getcwd_success);
    RUN_TEST(test_platform_getcwd_buffer_too_small);
    RUN_TEST(test_platform_getcwd_empty_buffer);
    RUN_TEST(test_platform_realpath_basic);
    RUN_TEST(test_platform_realpath_null_path_returns_null);
    RUN_TEST(test_platform_realpath_nonexistent_file);
    RUN_TEST(test_platform_realpath_relative_path_converted);

    /* Sleep tests */
    RUN_TEST(test_platform_sleep_ms_basic);
    RUN_TEST(test_platform_sleep_ms_zero);
    RUN_TEST(test_platform_sleep_ms_small_duration);
    RUN_TEST(test_platform_sleep_us_basic);
    RUN_TEST(test_platform_sleep_us_zero);
    RUN_TEST(test_platform_sleep_us_small_duration);

    /* Thread synchronization tests */
    RUN_TEST(test_platform_mutex_init_success);
    RUN_TEST(test_platform_mutex_init_null_mutex_returns_error);
    RUN_TEST(test_platform_mutex_lock_unlock_basic);
    RUN_TEST(test_platform_mutex_lock_unlock_multiple);
    RUN_TEST(test_platform_mutex_lock_unlock_race_condition);
    RUN_TEST(test_platform_mutex_destroy_after_use);

    /* Error handling tests */
    RUN_TEST(test_platform_get_last_error_after_success_returns_zero);
    RUN_TEST(test_platform_get_last_error_after_failure);
    RUN_TEST(test_platform_strerror_valid_error);
    RUN_TEST(test_platform_strerror_zero_returns_ok_message);
    RUN_TEST(test_platform_strerror_invalid_error_returns_message);

    /* String comparison tests */
    RUN_TEST(test_platform_stricmp_equal_strings);
    RUN_TEST(test_platform_stricmp_case_insensitive_equal);
    RUN_TEST(test_platform_stricmp_case_insensitive_equal_mixed);
    RUN_TEST(test_platform_stricmp_not_equal);
    RUN_TEST(test_platform_stricmp_different_case_not_equal);
    RUN_TEST(test_platform_stricmp_null_first_returns_nonzero);
    RUN_TEST(test_platform_stricmp_null_second_returns_nonzero);
    RUN_TEST(test_platform_stricmp_empty_strings);
    RUN_TEST(test_platform_stricmp_empty_vs_nonempty);
    RUN_TEST(test_platform_stricmp_numeric_strings);

    /* Edge case tests */
    RUN_TEST(test_platform_strncpy_s_exact_size);
    RUN_TEST(test_platform_strncpy_s_one_byte_over_dest_size);
    RUN_TEST(test_platform_strncat_s_dest_with_null_terminator);
    RUN_TEST(test_platform_strdup_very_long_string);
    RUN_TEST(test_platform_getcwd_absolute_path);
    RUN_TEST(test_platform_realpath_symlink_handling);

    /* Performance tests */
    RUN_TEST(test_platform_sleep_ms_performance);
    RUN_TEST(test_platform_mutex_performance);

    return UNITY_END();
}
