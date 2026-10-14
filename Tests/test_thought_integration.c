/**
 * @file test_thought_integration.c
 * @brief Integration test for THOUGHT application in SuperROM port for NEC PC-8201A
 *
 * This test exercises the complete workflow:
 * 1. Set 8201 model
 * 2. Load SUPNEC.bin SuperROM option ROM
 * 3. SUPER ROM should appear in the NEC menu
 * 4. Launch SUPER from menu
 * 5. From SUPER ROM menu, invoke THOUGHT (F3)
 * 6. Create an outline with one heading
 * 7. Save the outline
 * 8. Exit THOUGHT and SUPER ROM
 * 9. Re-launch SUPER ROM and THOUGHT
 * 10. Load the saved outline to verify it was persisted
 * 11. Confirm the outline content
 *
 * Note: THOUGHT does not generate lcdwrite events in VirtualT, so this test
 * verifies operation by checking that keystrokes are accepted and that
 * the save/load cycle completes without errors.
 */

#define _CRT_SECURE_NO_WARNINGS

#include "unity.h"
#include "platform_compat.h"
#include "vt_interact.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#endif

/* Test fixtures */
static vt_interact_handle_t handle = NULL;

/* Helper function to get VirtualT path from environment */
static const char* get_virtualt_path(void) {
    const char* path = getenv("VIRTUALT_PATH");
    if (!path) {
        #ifdef _WIN32
        _putenv("VIRTUALT_PATH=C:/Users/John/tools/VirtualT/");
        path = getenv("VIRTUALT_PATH");
        #endif
    }
    TEST_ASSERT_NOT_NULL_MESSAGE(path, "VIRTUALT_PATH environment variable must be set");
    return path;
}

/* Helper to check and print error */
static void check_error(const char* msg) {
    const char* error = vt_interact_get_error();
    if (error && error[0] != '\0') {
        printf("%s: Error: %s\n", msg, error);
    }
}

/* Helper to print directory area contents */
static void print_directory(const char* dir_buffer, int size) {
    printf("Directory bytes:\n");
    for (int i = 0; i < size; i++) {
        if (dir_buffer[i] == 0xFF) {
            printf("[%02X:EOF] ", (unsigned char)dir_buffer[i]);
        } else if (dir_buffer[i] >= 32 && dir_buffer[i] < 128) {
            printf("[%02X:%c] ", (unsigned char)dir_buffer[i], dir_buffer[i]);
        } else {
            printf("[%02X] ", (unsigned char)dir_buffer[i]);
        }
        if ((i + 1) % 11 == 0) printf("\n");
    }
    printf("\n");
    fflush(stdout);
}

/* Helper to read memory and verify it contains expected content */
/* This is a wrapper that uses vt_interact_verify_mem_contains internally */

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
}

/**
 * Test: Complete THOUGHT workflow integration for NEC PC-8201A SuperROM port
 *
 * This test exercises the full workflow of launching SuperROM on NEC,
 * running THOUGHT, creating an outline, saving it, and verifying
 * the saved content.
 *
 * Note: THOUGHT does not generate lcdwrite events in VirtualT, so we verify
 * operation by checking that keystrokes are accepted and by using
 * VirtualT's RAM save/load feature to verify file content.
 */
void test_thought_complete_workflow(void) {
    const char* vt_path = get_virtualt_path();

    printf("=== Test Start: THOUGHT Integration ===\n");
    fflush(stdout);

    /* Step 1: Create and launch VirtualT */
    printf("Step 1: Creating VirtualT handle...\n");
    fflush(stdout);
    printf("VIRTUALT_PATH = '%s'\n", vt_path);
    fflush(stdout);
    handle = vt_interact_create();
    TEST_ASSERT_NOT_NULL(handle);

    vt_interact_config_t config = {
        .virtualt_path = vt_path,
        .port = 0,  /* Auto-assign port */
        .headless = true,
        .startup_timeout_ms = 10000  /* Reduced from 30s */
    };

    printf("Step 2: Launching VirtualT...\n");
    fflush(stdout);
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_launch(handle, &config), vt_interact_get_error());
    check_error("After launch");
    printf("VirtualT launched successfully\n");
    fflush(stdout);

    printf("Step 3: Waiting for VirtualT to be ready...\n");
    printf("  Waiting up to 15 seconds...\n");
    fflush(stdout);
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_wait_for_ready(handle, 15000), vt_interact_get_error());
    printf("  VirtualT is ready!\n");
    fflush(stdout);
    check_error("After wait_for_ready");

    TEST_ASSERT_TRUE_MESSAGE(vt_interact_connect(handle), vt_interact_get_error());

    /* Step 2: Set model to PC-8201 */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_set_model(handle, "pc8201"), vt_interact_get_error());

    /* Clean up any existing TEST file from prior test runs */
    /* The VirtualT emulator remembers its file system state between invocations */
    printf("Cleaning up any existing TEST file...\n");
    fflush(stdout);

    /* Read directory to show current state */
    char dir_buffer[100];
    bool dir_read = vt_interact_read_memory(handle, 0xF84F, 88, dir_buffer, sizeof(dir_buffer));
    if (dir_read) {
        print_directory(dir_buffer, 88);
    }

    /* Use BASIC KILL command to delete the file if it exists */
    /* THOUGHT files may be stored as "TEST" or "TEST.CT" */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_launch_basic_from_menu(handle), vt_interact_get_error());
    vt_interact_type_line(handle, "KILL \"TEST\"");
    vt_interact_press_key(handle, "enter");
#ifdef _WIN32
    Sleep(200);
#else
    usleep(200000);
#endif
    vt_interact_type_line(handle, "KILL \"TEST.CT\"");
    vt_interact_press_key(handle, "enter");
#ifdef _WIN32
    Sleep(200);
#else
    usleep(200000);
#endif
    /* Exit BASIC back to main menu */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_exit_to_menu(handle), vt_interact_get_error());
#ifdef _WIN32
    Sleep(300);
#else
    usleep(300000);
#endif

    /* Verify TEST file is not present in directory */
    printf("Verifying cleanup...\n");
    fflush(stdout);
    dir_read = vt_interact_read_memory(handle, 0xF84F, 88, dir_buffer, sizeof(dir_buffer));
    if (dir_read) {
        print_directory(dir_buffer, 88);
        /* Verify no TEST file is present */
        bool test_found = false;
        for (int i = 0; i < 88 - 4; i++) {
            if (dir_buffer[i] == 'T' && dir_buffer[i+1] == 'E' &&
                dir_buffer[i+2] == 'S' && dir_buffer[i+3] == 'T') {
                test_found = true;
                break;
            }
        }
        TEST_ASSERT_TRUE_MESSAGE(!test_found, "TEST file still present in directory after cleanup");
    }

    printf("Cleanup complete, proceeding with test...\n");
    fflush(stdout);

    /* Step 3: Load the SUPNEC.bin SuperROM option ROM */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_load_rom(handle), vt_interact_get_error());

    /* Give the ROM time to initialize */
#ifdef _WIN32
    Sleep(300);
#else
    usleep(300000);
#endif

    /* Step 4: Install SUPER ROM by launching BASIC and running EXEC 62394 */
    /* First, navigate to BASIC in the menu and launch it */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_launch_basic_from_menu(handle), vt_interact_get_error());

    /* Wait for BASIC to start */
#ifdef _WIN32
    Sleep(300);
#else
    usleep(300000);
#endif

    /* Now run the SUPER ROM installation command */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_type_line(handle, "EXEC 62394"), vt_interact_get_error());
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "enter"), vt_interact_get_error());

    /* Wait for SUPER ROM to install */
#ifdef _WIN32
    Sleep(300);
#else
    usleep(300000);
#endif

    /* Exit back to the main menu */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_exit_to_menu(handle), vt_interact_get_error());

    /* Wait for main menu to load */
#ifdef _WIN32
    Sleep(300);
#else
    usleep(300000);
#endif

    /* Step 5: SUPER ROM should now appear in the NEC menu */
    /* Navigate to SUPER in the menu and launch it */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_menu_navigate(handle, 3, 0), vt_interact_get_error());

    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "enter"), vt_interact_get_error());

    /* Wait for SUPER ROM menu to appear */
#ifdef _WIN32
    Sleep(300);
#else
    usleep(300000);
#endif

    /* Step 6: From SUPER ROM menu, invoke THOUGHT (F3) */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "f3"), vt_interact_get_error());

    /* Wait for THOUGHT to start - THOUGHT shows "New Filename:" prompt */
    /* Note: THOUGHT does not generate lcdwrite events in VirtualT */
    printf("Step 15: Waiting for THOUGHT to start (no LCD events expected)...\n");
#ifdef _WIN32
    platform_sleep_ms(500);
#else
    usleep(500000);
#endif

    /* Verify we're in THOUGHT by typing a filename - THOUGHT should accept it */
    /* THOUGHT's "New Filename:" prompt accepts text input */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_type_text(handle, "TEST", false), vt_interact_get_error());

    /* Press ENTER to confirm the filename */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "enter"), vt_interact_get_error());
    check_error("After press_key enter");

    /* Wait for THOUGHT to create the outline */
#ifdef _WIN32
    Sleep(300);
#else
    usleep(300000);
#endif

    /* In Create mode, type the first headline */
    /* THOUGHT shows a ? for create mode and accepts text input */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_type_line(handle, "My First Outline"), vt_interact_get_error());

    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "enter"), vt_interact_get_error());

    /* Wait for the line to be processed */
#ifdef _WIN32
    Sleep(300);
#else
    usleep(300000);
#endif

    /* Step 7: Save the outline */
    /* In THOUGHT, save is done via F3 (Save) */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "f3"), vt_interact_get_error());

    /* Wait for save dialog */
#ifdef _WIN32
    Sleep(300);
#else
    usleep(300000);
#endif

    /* The filename should already be filled in, just press ENTER */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "enter"), vt_interact_get_error());

    /* Wait for save to complete */
#ifdef _WIN32
    Sleep(300);
#else
    usleep(300000);
#endif

    /* Step 8: Exit THOUGHT back to SUPER ROM menu */
    /* Use F8 to exit to SUPER ROM menu */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "shift-f5"), vt_interact_get_error());
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "shift-f5"), vt_interact_get_error());
    printf("  F8 pressed (exit THOUGHT)!\n");
    fflush(stdout);
    check_error("After press_key shift-f5");

    /* Wait for exit to complete */
#ifdef _WIN32
    Sleep(300);
#else
    usleep(300000);
#endif

    /* Step 9: Exit SUPER ROM to NEC main menu */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "shift-f5"), vt_interact_get_error());

    /* Wait for menu to load */
#ifdef _WIN32
    Sleep(300);
#else
    usleep(300000);
#endif

    /* Step 11: Re-open the outline with THOUGHT */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_menu_navigate(handle, 3, 0), vt_interact_get_error());

    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "enter"), vt_interact_get_error());

    /* Wait for SUPER ROM menu */
#ifdef _WIN32
    Sleep(300);
#else
    usleep(300000);
#endif

    /* Invoke THOUGHT */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "f3"), vt_interact_get_error());

    /* Wait for THOUGHT */
#ifdef _WIN32
    Sleep(300);
#else
    usleep(300000);
#endif

    /* THOUGHT should now show "New Filename:" and we type TEST to load it */
    /* In THOUGHT's New Filename prompt, type "TEST" and ENTER to load */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_type_text(handle, "TEST", true), vt_interact_get_error());
    printf("  TEST typed (load)!\n");
    fflush(stdout);
    check_error("After type_text TEST for loading");

    /* Wait for outline to load */
#ifdef _WIN32
    Sleep(300);
#else
    usleep(300000);
#endif

    /* Verify the outline content by reading screen memory
     * After loading an outline, THOUGHT displays the outline entries
     */
    printf("Step 28a: Verifying outline content from screen memory...\n");
    fflush(stdout);

    char screen_buffer[320];
    bool success = vt_interact_read_memory(handle, 0xFD00, 320, screen_buffer, sizeof(screen_buffer));

    /* The outline should contain the heading we created */
    TEST_ASSERT_TRUE_MESSAGE(success, "Failed to read screen memory");
    TEST_ASSERT_TRUE_MESSAGE(
        strstr(screen_buffer, "My First Outline") != NULL,
        "Expected heading 'My First Outline' not found in loaded outline"
    );
    printf("Successfully verified outline content!\n");

    /* Type F8 to exit THOUGHT */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "shift-f5"), vt_interact_get_error());

    /* Wait for menu */
#ifdef _WIN32
    Sleep(300);
#else
    usleep(300000);
#endif

    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "shift-f5"), vt_interact_get_error());

    /* Wait for menu */
#ifdef _WIN32
    Sleep(300);
#else
    usleep(300000);
#endif

    printf("=== Test Complete ===\n");

    /* Cleanup */
    vt_interact_disconnect(handle);
}

int main(int argc, char** argv) {
    const char* error;

    /* Parse command line arguments for test filtering */
    UnityParseOptions(argc, argv);

    UNITY_BEGIN();

    /* Run integration test */
    RUN_TEST(test_thought_complete_workflow);

    /* Print error info if tests failed */
    error = vt_interact_get_error();
    if (error && error[0] != '\0') {
        printf("Final Error: %s\n", error);
    }

    return UNITY_END();
}
