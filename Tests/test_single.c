/**
 * @file test_single.c
 * @brief Single test to debug hanging issue
 */

#include "unity.h"
#include "vt_interact.h"
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#endif

void setUp(void) {
    // Empty
}

void tearDown(void) {
    // Empty
}

void test_launch_and_terminate(void) {
    const char* vt_path = getenv("VIRTUALT_PATH");
    if (!vt_path) {
        vt_path = "C:/Users/John/tools/VirtualT/VirtualT.exe";
    }

    vt_interact_handle_t handle = vt_interact_create();
    TEST_ASSERT_NOT_NULL(handle);

    vt_interact_config_t config = {
        .virtualt_path = vt_path,
        .port = 0,
        .headless = true,
        .startup_timeout_ms = 30000
    };

    TEST_ASSERT_TRUE(vt_interact_launch(handle, &config));
    TEST_ASSERT_TRUE(vt_interact_wait_for_ready(handle, 30000));
    TEST_ASSERT_TRUE(vt_interact_connect(handle));

    /* Set model */
    TEST_ASSERT_TRUE(vt_interact_set_model(handle, "pc8201"));

    /* Load ROM */
    TEST_ASSERT_TRUE(vt_interact_load_rom(handle));

    /* Give it time to load */
    Sleep(1000);

    /* Terminate */
    TEST_ASSERT_TRUE(vt_interact_terminate(handle));
    TEST_ASSERT_FALSE(vt_interact_is_running(handle));

    vt_interact_destroy(handle);
}

int main(void) {
    setUp();
    test_launch_and_terminate();
    tearDown();
    return 0;
}
