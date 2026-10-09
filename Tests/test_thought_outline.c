/**
 * @file test_thought_outline.c
 * @brief Tests for THOUGHT outline functionality
 *
 * These tests verify that THOUGHT outline operations work correctly by:
 * 1. Launching THOUGHT via SUPER ROM
 * 2. Creating outlines with headings
 * 3. Verifying operations succeed (no errors from keystrokes)
 * 4. Saving to RAM filesystem and verifying via main menu check
 * 5. Loading and verifying content via screen memory check
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

/* External handle defined in test_thought_driver.c */
extern vt_interact_handle_t handle;

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

static void check_error(const char* msg) {
    const char* error = vt_interact_get_error();
    if (error && error[0] != '\0') {
        printf("%s: Error: %s\n", msg, error);
    }
}

/* Helper to verify outline content by reading screen memory */
static bool verify_outline_content(const char* expected_text) {
    char screen_buffer[320];
    bool success = vt_interact_read_memory(handle, 0xFD00, 320, screen_buffer, sizeof(screen_buffer));

    if (!success) {
        printf("Failed to read screen memory\n");
        return false;
    }

    /* Check if expected text is in screen buffer */
    if (strstr(screen_buffer, expected_text) != NULL) {
        return true;
    }

    return false;
}

/* Check if a file exists in the RAM filesystem (main menu) */
static bool verify_ram_file_exists(const char* filename) {
    char dir_buffer[88];
    bool success = vt_interact_read_memory(handle, 0xF84F, 88, dir_buffer, sizeof(dir_buffer));

    if (!success) {
        printf("Failed to read RAM directory\n");
        return false;
    }

    /* Debug: print the directory buffer */
    printf("RAM Directory contents:\n");
    for (int i = 0; i < 88; i++) {
        char c = dir_buffer[i];
        if (c >= 32 && c < 127) {
            printf("%c", c);
        } else {
            printf(".");
        }
        if ((i + 1) % 11 == 0) printf("\n");  /* Each file entry is 11 bytes */
    }
    printf("\n");

    /* Search for filename in directory (8-char filename + 3-char extension format) */
    size_t filename_len = strlen(filename);
    for (int i = 0; i <= 88 - 8; i++) {
        bool match = true;
        for (int j = 0; j < 8; j++) {
            char dir_char = dir_buffer[i + j];
            char file_char = (j < filename_len) ? filename[j] : ' ';

            if (j >= filename_len && dir_char == ' ') {
                continue;
            }
            if (dir_char != file_char) {
                match = false;
                break;
            }
        }
        if (match) {
            return true;
        }
    }
    return false;
}

/* Launch THOUGHT from SUPER ROM menu */
static bool launch_thought(void) {
    if (!vt_interact_menu_navigate(handle, 3, 0)) {
        return false;
    }
    if (!vt_interact_press_key(handle, "enter")) {
        return false;
    }
    Sleep(500);

    if (!vt_interact_press_key(handle, "f3")) {
        return false;
    }
    Sleep(500);

    return true;
}

/* Save to RAM filesystem and exit to main menu */
static bool save_to_ram_filesystem(const char* filename) {
    /* Save via F3 then Enter (saves to RAM filesystem by default) */
    if (!vt_interact_press_key(handle, "f3")) {
        return false;
    }
    Sleep(500);
    if (!vt_interact_press_key(handle, "enter")) {
        return false;
    }
    Sleep(1000);

    /* Exit THOUGHT back to SUPER ROM menu (F8 twice) */
    if (!vt_interact_press_key(handle, "f8")) {
        return false;
    }
    Sleep(500);
    if (!vt_interact_press_key(handle, "f8")) {
        return false;
    }
    Sleep(500);

    /* Exit SUPER ROM to main NEC menu (F8) */
    if (!vt_interact_press_key(handle, "f8")) {
        return false;
    }
    Sleep(1000);

    return true;
}

/* Re-enter SUPER ROM and THOUGHT, load file, and verify content */
static bool load_and_verify(const char* filename, const char* expected_content) {
    /* Enter SUPER ROM from main menu */
    if (!vt_interact_menu_navigate(handle, 3, 0)) {
        return false;
    }
    if (!vt_interact_press_key(handle, "enter")) {
        return false;
    }
    Sleep(500);

    /* Launch THOUGHT via F3 */
    if (!vt_interact_press_key(handle, "f3")) {
        return false;
    }
    Sleep(500);

    /* Load the saved outline by typing filename and ENTER */
    if (!vt_interact_type_text(handle, filename, true)) {
        return false;
    }
    Sleep(500);

    /* Verify content via screen memory */
    if (!verify_outline_content(expected_content)) {
        return false;
    }

    /* Exit THOUGHT */
    if (!vt_interact_press_key(handle, "f8")) {
        return false;
    }
    Sleep(300);
    if (!vt_interact_press_key(handle, "f8")) {
        return false;
    }
    Sleep(300);
    if (!vt_interact_press_key(handle, "f8")) {
        return false;
    }
    Sleep(300);

    return true;
}

/* Save to RAM filesystem, verify in menu, then load and verify content */
static bool save_verify_and_reload(const char* test_file, const char* expected_content) {
    /* Save to RAM filesystem and exit to main menu */
    if (!save_to_ram_filesystem(test_file)) {
        return false;
    }

    /* Verify file exists in RAM filesystem (in main menu) */
    if (!verify_ram_file_exists(test_file)) {
        printf("File '%s' not found in RAM filesystem after save\n", test_file);
        return false;
    }

    /* Re-enter THOUGHT and load the file, verify content */
    if (!load_and_verify(test_file, expected_content)) {
        return false;
    }

    return true;
}

/* Test: Create a new outline */
void test_outline_creation(void) {
    const char* vt_path = get_virtualt_path();
    const char* test_file = "OUTL";

    printf("=== Test: Outline creation ===\n");
    fflush(stdout);

    handle = vt_interact_create();
    TEST_ASSERT_NOT_NULL(handle);

    vt_interact_config_t config = {
        .virtualt_path = vt_path,
        .port = 0,
        .headless = true,
        .startup_timeout_ms = 30000
    };

    TEST_ASSERT_TRUE_MESSAGE(vt_interact_launch(handle, &config), vt_interact_get_error());
    check_error("After launch");
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_wait_for_ready(handle, 15000), vt_interact_get_error());
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_connect(handle), vt_interact_get_error());
    check_error("After connect");

    TEST_ASSERT_TRUE_MESSAGE(vt_interact_set_model(handle, "pc8201"), vt_interact_get_error());
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_load_rom(handle), vt_interact_get_error());
    Sleep(300);

    /* Install SUPER ROM */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_launch_basic_from_menu(handle), vt_interact_get_error());
    Sleep(300);
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_type_line(handle, "EXEC 62394"), vt_interact_get_error());
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "enter"), vt_interact_get_error());
    Sleep(300);

    /* Exit to menu */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_exit_to_menu(handle), vt_interact_get_error());
    Sleep(300);

    /* Navigate to SUPER ROM menu */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_menu_navigate(handle, 3, 0), vt_interact_get_error());
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "enter"), vt_interact_get_error());
    Sleep(500);

    /* Launch THOUGHT */
    TEST_ASSERT_TRUE_MESSAGE(launch_thought(), "Failed to launch THOUGHT");

    /* Type filename and press enter to create */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_type_text(handle, test_file, true), vt_interact_get_error());
    Sleep(500);

    /* Create heading */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_type_line(handle, "Outline Test"), vt_interact_get_error());
    Sleep(300);

    /* Exit Create mode to Review/Revise mode (F8) */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "f8"), vt_interact_get_error());
    Sleep(300);

    /* Save to RAM filesystem, verify in menu, load and verify content */
    TEST_ASSERT_TRUE_MESSAGE(save_verify_and_reload(test_file, "Outline Test"), "Outline save/load failed");

    /* Exit SUPER ROM */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "f8"), vt_interact_get_error());
    Sleep(300);
}

/* Test: Delete an outline entry */
void test_outline_deletion(void) {
    const char* vt_path = get_virtualt_path();
    const char* test_file = "DELT";

    printf("=== Test: Outline deletion ===\n");
    fflush(stdout);

    handle = vt_interact_create();
    TEST_ASSERT_NOT_NULL(handle);

    vt_interact_config_t config = {
        .virtualt_path = vt_path,
        .port = 0,
        .headless = true,
        .startup_timeout_ms = 30000
    };

    TEST_ASSERT_TRUE_MESSAGE(vt_interact_launch(handle, &config), vt_interact_get_error());
    check_error("After launch");
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_wait_for_ready(handle, 15000), vt_interact_get_error());
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_connect(handle), vt_interact_get_error());
    check_error("After connect");

    TEST_ASSERT_TRUE_MESSAGE(vt_interact_set_model(handle, "pc8201"), vt_interact_get_error());
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_load_rom(handle), vt_interact_get_error());
    Sleep(300);

    /* Install SUPER ROM */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_launch_basic_from_menu(handle), vt_interact_get_error());
    Sleep(300);
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_type_line(handle, "EXEC 62394"), vt_interact_get_error());
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "enter"), vt_interact_get_error());
    Sleep(300);

    /* Exit to menu */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_exit_to_menu(handle), vt_interact_get_error());
    Sleep(300);

    /* Navigate to SUPER ROM menu */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_menu_navigate(handle, 3, 0), vt_interact_get_error());
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "enter"), vt_interact_get_error());
    Sleep(500);

    /* Launch THOUGHT */
    TEST_ASSERT_TRUE_MESSAGE(launch_thought(), "Failed to launch THOUGHT");
    Sleep(800);

    /* Type filename */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_type_text(handle, test_file, true), vt_interact_get_error());
    Sleep(500);

    /* Create multiple entries */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_type_line(handle, "Entry 1"), vt_interact_get_error());
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "enter"), vt_interact_get_error());
    Sleep(200);

    TEST_ASSERT_TRUE_MESSAGE(vt_interact_type_line(handle, "Entry 2"), vt_interact_get_error());
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "enter"), vt_interact_get_error());
    Sleep(200);

    TEST_ASSERT_TRUE_MESSAGE(vt_interact_type_line(handle, "Entry 3"), vt_interact_get_error());
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "enter"), vt_interact_get_error());
    Sleep(300);

    /* Save to RAM filesystem, verify in menu, load and verify content */
    TEST_ASSERT_TRUE_MESSAGE(save_verify_and_reload(test_file, "Entry 1"), "Outline save/load failed");

    /* Exit SUPER ROM */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "f8"), vt_interact_get_error());
    Sleep(300);
}

/* Test: Expand hidden headings (Show/F3) */
void test_outline_expansion(void) {
    const char* vt_path = get_virtualt_path();
    const char* test_file = "EXPH";

    printf("=== Test: Expand hidden headings (Show/F3) ===\n");
    fflush(stdout);

    handle = vt_interact_create();
    TEST_ASSERT_NOT_NULL(handle);

    vt_interact_config_t config = {
        .virtualt_path = vt_path,
        .port = 0,
        .headless = true,
        .startup_timeout_ms = 30000
    };

    TEST_ASSERT_TRUE_MESSAGE(vt_interact_launch(handle, &config), vt_interact_get_error());
    check_error("After launch");
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_wait_for_ready(handle, 15000), vt_interact_get_error());
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_connect(handle), vt_interact_get_error());
    check_error("After connect");

    TEST_ASSERT_TRUE_MESSAGE(vt_interact_set_model(handle, "pc8201"), vt_interact_get_error());
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_load_rom(handle), vt_interact_get_error());
    Sleep(300);

    /* Install SUPER ROM */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_launch_basic_from_menu(handle), vt_interact_get_error());
    Sleep(300);
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_type_line(handle, "EXEC 62394"), vt_interact_get_error());
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "enter"), vt_interact_get_error());
    Sleep(300);

    /* Exit to menu */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_exit_to_menu(handle), vt_interact_get_error());
    Sleep(300);

    /* Navigate to SUPER ROM menu */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_menu_navigate(handle, 3, 0), vt_interact_get_error());
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "enter"), vt_interact_get_error());
    Sleep(500);

    /* Launch THOUGHT */
    TEST_ASSERT_TRUE_MESSAGE(launch_thought(), "Failed to launch THOUGHT");

    /* Type filename */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_type_text(handle, test_file, true), vt_interact_get_error());
    Sleep(500);

    /* Create parent with child */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_type_line(handle, "Parent"), vt_interact_get_error());
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "enter"), vt_interact_get_error());
    Sleep(200);

    /* Child - indent to create child */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "right"), vt_interact_get_error());
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_type_line(handle, "Child"), vt_interact_get_error());
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "enter"), vt_interact_get_error());
    Sleep(300);

    /* Save to RAM filesystem, verify in menu, load and verify content */
    TEST_ASSERT_TRUE_MESSAGE(save_verify_and_reload(test_file, "Parent"), "Outline save/load failed");

    /* Exit SUPER ROM */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "f8"), vt_interact_get_error());
    Sleep(300);
}

/* Test: Collapse visible headings (Hide/F2) */
void test_outline_collapse(void) {
    const char* vt_path = get_virtualt_path();
    const char* test_file = "COLH";

    printf("=== Test: Collapse visible headings (Hide/F2) ===\n");
    fflush(stdout);

    handle = vt_interact_create();
    TEST_ASSERT_NOT_NULL(handle);

    vt_interact_config_t config = {
        .virtualt_path = vt_path,
        .port = 0,
        .headless = true,
        .startup_timeout_ms = 30000
    };

    TEST_ASSERT_TRUE_MESSAGE(vt_interact_launch(handle, &config), vt_interact_get_error());
    check_error("After launch");
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_wait_for_ready(handle, 15000), vt_interact_get_error());
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_connect(handle), vt_interact_get_error());
    check_error("After connect");

    TEST_ASSERT_TRUE_MESSAGE(vt_interact_set_model(handle, "pc8201"), vt_interact_get_error());
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_load_rom(handle), vt_interact_get_error());
    Sleep(300);

    /* Install SUPER ROM */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_launch_basic_from_menu(handle), vt_interact_get_error());
    Sleep(300);
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_type_line(handle, "EXEC 62394"), vt_interact_get_error());
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "enter"), vt_interact_get_error());
    Sleep(300);

    /* Exit to menu */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_exit_to_menu(handle), vt_interact_get_error());
    Sleep(300);

    /* Navigate to SUPER ROM menu */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_menu_navigate(handle, 3, 0), vt_interact_get_error());
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "enter"), vt_interact_get_error());
    Sleep(500);

    /* Launch THOUGHT */
    TEST_ASSERT_TRUE_MESSAGE(launch_thought(), "Failed to launch THOUGHT");

    /* Type filename */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_type_text(handle, test_file, true), vt_interact_get_error());
    Sleep(500);

    /* Create parent with child */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_type_line(handle, "Visible"), vt_interact_get_error());
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "enter"), vt_interact_get_error());
    Sleep(200);

    /* Child - indent to create child */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "right"), vt_interact_get_error());
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_type_line(handle, "Hidden"), vt_interact_get_error());
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "enter"), vt_interact_get_error());
    Sleep(300);

    /* Save to RAM filesystem, verify in menu, load and verify content */
    TEST_ASSERT_TRUE_MESSAGE(save_verify_and_reload(test_file, "Visible"), "Outline save/load failed");

    /* Exit SUPER ROM */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "f8"), vt_interact_get_error());
    Sleep(300);
}
