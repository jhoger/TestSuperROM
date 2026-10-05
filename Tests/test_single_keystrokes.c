/**
 * @file test_single_keystrokes.c
 * @brief Test to debug keystrokes hanging issue
 */

#define _CRT_SECURE_NO_WARNINGS

#include "unity.h"
#include "vt_interact.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#endif

static vt_interact_handle_t handle = NULL;

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

static const char* get_virtualt_path(void) {
    const char* path = getenv("VIRTUALT_PATH");
    if (!path) {
        #ifdef _WIN32
        _putenv("VIRTUALT_PATH=C:/Users/John/tools/VirtualT/VirtualT.exe");
        path = getenv("VIRTUALT_PATH");
        #endif
    }
    TEST_ASSERT_NOT_NULL_MESSAGE(path, "VIRTUALT_PATH environment variable must be set");
    return path;
}

void test_keystrokes_simple(void) {
    printf("DEBUG: Creating handle\n");
    handle = vt_interact_create();
    TEST_ASSERT_NOT_NULL(handle);

    const char* vt_path = get_virtualt_path();

    vt_interact_config_t config = {
        .virtualt_path = vt_path,
        .port = 0,
        .headless = true,
        .startup_timeout_ms = 30000
    };

    printf("DEBUG: Launching\n");
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_launch(handle, &config), vt_interact_get_error());
    printf("DEBUG: Waiting for ready\n");
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_wait_for_ready(handle, 15000), vt_interact_get_error());
    printf("DEBUG: Connecting\n");
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_connect(handle), vt_interact_get_error());

    printf("DEBUG: Setting model\n");
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_set_model(handle, "pc8201"), vt_interact_get_error());
    printf("DEBUG: Loading ROM\n");
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_load_rom(handle), vt_interact_get_error());
    printf("DEBUG: Launching BASIC\n");
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_launch_basic_from_menu(handle), vt_interact_get_error());

    printf("DEBUG: Sending keystrokes\n");
    const char* keys[] = {"T", "E", "S", "T"};
    int error_code = 0;

    bool result = vt_interact_send_keystrokes(handle, keys, 4, 1000, "TEST", true, &error_code);

    printf("DEBUG: Result = %d, error = %d, error_msg = %s\n", result, error_code, vt_interact_get_error());
    TEST_ASSERT_TRUE_MESSAGE(result, vt_interact_get_error());
    TEST_ASSERT_EQUAL_INT(VT_INTERACT_SUCCESS, error_code);

    vt_interact_exit_to_menu(handle);
    vt_interact_disconnect(handle);
}

int main(void) {
    setUp();
    test_keystrokes_simple();
    tearDown();
    printf("All tests passed!\n");
    return 0;
}
