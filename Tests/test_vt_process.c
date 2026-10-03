/**
 * @file test_vt_process.c
 * @brief Unit tests for vt_process library
 */

#include "unity.h"
#include "vt_process.h"
#include "vt_socket.h"
#include <stdlib.h>

/* Test fixtures */
static vt_process_handle_t process_handle = NULL;

/* Setup and teardown for Unity */
void setUp(void) {
    process_handle = vt_process_create();
    TEST_ASSERT_NOT_NULL(process_handle);
}

void tearDown(void) {
    if (process_handle) {
        vt_process_terminate(process_handle);
        vt_process_destroy(process_handle);
        process_handle = NULL;
    }
}

/* Simple test to verify build works */
void test_basic(void) {
    vt_process_handle_t handle = vt_process_create();
    TEST_ASSERT_NOT_NULL(handle);
    vt_process_destroy(handle);
}

/* Test launch and terminate - uses VIRTUALT_PATH environment variable */
void test_launch_terminate(void) {
    /* Set VIRTUALT_PATH if not already set */
    const char* vt_path = getenv("VIRTUALT_PATH");
    TEST_ASSERT_NOT_NULL_MESSAGE(vt_path, "VIRTUALT_PATH environment variable must be set");
    
    vt_process_handle_t handle = vt_process_create();
    TEST_ASSERT_NOT_NULL(handle);
    
    vt_process_config_t config = {
        .virtualt_path = vt_path,
        .port = 0,  /* Auto-assign port */
        .headless = true,
        .startup_timeout_ms = 30000
    };
    
    TEST_ASSERT_TRUE(vt_process_launch(handle, &config));
    TEST_ASSERT_TRUE(vt_process_is_running(handle));
    
    /* Give process time to start */
    TEST_ASSERT_TRUE(vt_process_wait_for_ready(handle, 15000));
    
    vt_process_terminate(handle);
    TEST_ASSERT_FALSE(vt_process_is_running(handle));
    
    vt_process_destroy(handle);
}

int main(void) {
    const char* error;
    
    UNITY_BEGIN();
    RUN_TEST(test_basic);
    RUN_TEST(test_launch_terminate);
    
    /* Print error info if test failed */
    error = vt_process_get_error();
    if (error && error[0] != '\0') {
        printf("Error: %s\n", error);
    }
    
    return UNITY_END();
}