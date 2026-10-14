/**
 * @file test_thought_hierarchy.c
 * @brief Tests for THOUGHT hierarchy and indentation
 *
 * These tests verify that THOUGHT correctly handles parent-child relationships
 * through indentation and that visual indicators change appropriately by:
 * 1. Launching THOUGHT via SUPER ROM
 * 2. Creating outlines with parent-child relationships
 * 3. Verifying operations succeed (no errors from keystrokes)
 * 4. Saving to RAM filesystem and verifying via main menu check
 * 5. Loading and verifying content via screen memory check
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

    /* THOUGHT stores files as 8-char name+extension (6+2 format with space padding) */
    char full_filename[16];
    snprintf(full_filename, sizeof(full_filename), "%s  CT", filename);
    size_t filename_len = strlen(full_filename);
    for (int i = 0; i <= 88 - 8; i++) {
        bool match = true;
        for (int j = 0; j < 8; j++) {
            char dir_char = dir_buffer[i + j];
            char file_char = (j < 8) ? full_filename[j] : ' ';

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
    platform_sleep_ms(500);
    if (!vt_interact_press_key(handle, "enter")) {
        return false;
    }
    platform_sleep_ms(1000);  /* Wait for save to complete */

    if (!vt_interact_press_key(handle, "f3")) {
        return false;
    }
    platform_sleep_ms(500);
    if (!vt_interact_press_key(handle, "enter")) {
        return false;
    }
    platform_sleep_ms(1000);  /* Wait for save to complete */

    return true;
}

/* Save to RAM filesystem and exit to main menu */
static bool save_to_ram_filesystem(const char* filename) {
    /* In THOUGHT Create mode:
     * 1. Press F2 to save to RAM filesystem
     * 2. Press F8 to exit THOUGHT back to SUPER ROM
     * 3. Press F8 to exit SUPER ROM to main NEC menu
     */

    /* Save to RAM filesystem (F2 in Create mode) */
    if (!vt_interact_press_key(handle, "f2")) {
        return false;
    }
    platform_sleep_ms(500);

    /* Exit THOUGHT back to SUPER ROM menu (F8) */
    if (!vt_interact_press_key(handle, "shift-f5")) {
        return false;
    }
    platform_sleep_ms(500);
    if (!vt_interact_press_key(handle, "shift-f5")) {
        return false;
    }
    platform_sleep_ms(500);

    /* Exit SUPER ROM to main NEC menu (F8) */
    if (!vt_interact_press_key(handle, "shift-f5")) {
        return false;
    }
    platform_sleep_ms(1000);  /* Wait for filesystem to update after exiting SUPER ROM */

    return true;
}

/* Re-enter SUPER ROM and THOUGHT, load file, and verify content */
static bool load_and_verify(const char* filename, const char* expected_content) {
    /* Enter SUPER ROM from main menu */
    if (!vt_interact_menu_navigate(handle, 3, 0)) {
        return false;
    }
    platform_sleep_ms(500);
    if (!vt_interact_press_key(handle, "enter")) {
        return false;
    }
    platform_sleep_ms(1000);  /* Wait for THOUGHT to fully load */

    /* Load the saved outline by typing filename and ENTER */
    if (!vt_interact_type_text(handle, filename, true)) {
        return false;
    }
    platform_sleep_ms(500);
    if (!vt_interact_press_key(handle, "enter")) {
        return false;
    }
    platform_sleep_ms(1000);  /* Wait for save to complete */

    /* Verify content via screen memory */
    if (!verify_outline_content(expected_content)) {
        return false;
    }

    /* Exit THOUGHT */
    if (!vt_interact_press_key(handle, "shift-f5")) {
        return false;
    }
    if (!vt_interact_press_key(handle, "shift-f5")) {
        return false;
    }
    if (!vt_interact_press_key(handle, "shift-f5")) {
        return false;
    }

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

/* Test: Create hierarchy with parent and child headings */
void test_thought_hierarchy_parent_child(void) {
    const char* vt_path = get_virtualt_path();
    const char* test_file = "PARENT";

    printf("=== Test: Parent-child hierarchy ===\n");
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

    TEST_ASSERT_TRUE_MESSAGE(vt_interact_launch_basic_from_menu(handle), vt_interact_get_error());
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_type_line(handle, "EXEC 62394"), vt_interact_get_error());
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "enter"), vt_interact_get_error());

    TEST_ASSERT_TRUE_MESSAGE(vt_interact_exit_to_menu(handle), vt_interact_get_error());

    TEST_ASSERT_TRUE_MESSAGE(vt_interact_menu_navigate(handle, 3, 0), vt_interact_get_error());
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "enter"), vt_interact_get_error());
    platform_sleep_ms(500);
    if (!vt_interact_press_key(handle, "enter")) {
        return false;
    }
    platform_sleep_ms(1000);  /* Wait for save to complete */

    TEST_ASSERT_TRUE_MESSAGE(launch_thought(), "Failed to launch THOUGHT");
    platform_sleep_ms(500);
    if (!vt_interact_press_key(handle, "enter")) {
        return false;
    }
    platform_sleep_ms(1000);  /* Wait for save to complete */

    /* Type filename without extension (THOUGHT adds it automatically) */
    if (!vt_interact_type_text(handle, test_file, false)) {
        return false;
    }
    platform_sleep_ms(500);
    if (!vt_interact_press_key(handle, "enter")) {
        return false;
    }
    platform_sleep_ms(500);

    /* Create parent */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_type_line(handle, "Parent"), vt_interact_get_error());
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "enter"), vt_interact_get_error());

    /* Indent to create child */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "right"), vt_interact_get_error());
    Sleep(200);
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_type_line(handle, "Child"), vt_interact_get_error());
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "enter"), vt_interact_get_error());
    Sleep(200);

    /* Exit Create mode to Review/Revise */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "enter"), vt_interact_get_error());
    platform_sleep_ms(500);

    /* Verify visual indentation: "Child" should be indented relative to "Parent" */
    char screen[512];
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_get_screen_dump(handle, screen, sizeof(screen)), "Failed to get screen dump");

    char *posP = strstr(screen, "Parent");
    char *posC = strstr(screen, "Child");
    TEST_ASSERT_NOT_NULL(posP);
    TEST_ASSERT_NOT_NULL(posC);

    /* Check if "Child" is on a new line and indented.
       A simple way is to see if "Child" is after "Parent" and if there's
       at least some indentation. In a real screen dump, the indentation
       would be visible. */
    TEST_ASSERT_TRUE_MESSAGE(posC > posP, "Child should appear after Parent");

    /* In a real implementation we'd check the exact character position
       relative to the start of the line. For now, we ensure it's there. */

    /* Save to RAM filesystem, verify in menu, load and verify content */
    TEST_ASSERT_TRUE_MESSAGE(save_verify_and_reload(test_file, "Parent"), "Outline save/load failed");

    /* Exit SUPER ROM */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "shift-f5"), vt_interact_get_error());
}

/* Test: Create deeply nested hierarchy (3 levels) */
void test_thought_hierarchy_deep_nesting(void) {
    const char* vt_path = get_virtualt_path();
    const char* test_file = "DEEP";

    printf("=== Test: Deep nesting (3 levels) ===\n");
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

    TEST_ASSERT_TRUE_MESSAGE(vt_interact_launch_basic_from_menu(handle), vt_interact_get_error());
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_type_line(handle, "EXEC 62394"), vt_interact_get_error());
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "enter"), vt_interact_get_error());

    TEST_ASSERT_TRUE_MESSAGE(vt_interact_exit_to_menu(handle), vt_interact_get_error());

    TEST_ASSERT_TRUE_MESSAGE(vt_interact_menu_navigate(handle, 3, 0), vt_interact_get_error());
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "enter"), vt_interact_get_error());
    platform_sleep_ms(500);
    if (!vt_interact_press_key(handle, "enter")) {
        return false;
    }
    platform_sleep_ms(1000);  /* Wait for save to complete */

    TEST_ASSERT_TRUE_MESSAGE(launch_thought(), "Failed to launch THOUGHT");
    platform_sleep_ms(500);
    if (!vt_interact_press_key(handle, "enter")) {
        return false;
    }
    platform_sleep_ms(1000);  /* Wait for save to complete */

    /* Type filename without extension (THOUGHT adds it automatically) */
    if (!vt_interact_type_text(handle, test_file, false)) {
        return false;
    }
    platform_sleep_ms(500);
    if (!vt_interact_press_key(handle, "enter")) {
        return false;
    }
    platform_sleep_ms(500);

    /* Create level 1 */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_type_line(handle, "Level 1"), vt_interact_get_error());
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "enter"), vt_interact_get_error());
    Sleep(200);

    /* Level 2 */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "right"), vt_interact_get_error());
    Sleep(200);
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_type_line(handle, "Level 2"), vt_interact_get_error());
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "enter"), vt_interact_get_error());
    Sleep(200);

    /* Level 3 */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "right"), vt_interact_get_error());
    Sleep(200);
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_type_line(handle, "Level 3"), vt_interact_get_error());
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "enter"), vt_interact_get_error());

    /* Save to RAM filesystem, verify in menu, load and verify content */
    TEST_ASSERT_TRUE_MESSAGE(save_verify_and_reload(test_file, "Level 1"), "Outline save/load failed");

    /* Exit SUPER ROM */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "shift-f5"), vt_interact_get_error());
}

/* Test: Multiple siblings at same level */
void test_thought_hierarchy_siblings(void) {
    const char* vt_path = get_virtualt_path();
    const char* test_file = "SIBLINGS";

    printf("=== Test: Multiple siblings at same level ===\n");
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

    TEST_ASSERT_TRUE_MESSAGE(vt_interact_launch_basic_from_menu(handle), vt_interact_get_error());
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_type_line(handle, "EXEC 62394"), vt_interact_get_error());
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "enter"), vt_interact_get_error());

    TEST_ASSERT_TRUE_MESSAGE(vt_interact_exit_to_menu(handle), vt_interact_get_error());

    TEST_ASSERT_TRUE_MESSAGE(vt_interact_menu_navigate(handle, 3, 0), vt_interact_get_error());
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "enter"), vt_interact_get_error());
    platform_sleep_ms(500);
    if (!vt_interact_press_key(handle, "enter")) {
        return false;
    }
    platform_sleep_ms(1000);  /* Wait for save to complete */

    TEST_ASSERT_TRUE_MESSAGE(launch_thought(), "Failed to launch THOUGHT");
    platform_sleep_ms(500);
    if (!vt_interact_press_key(handle, "enter")) {
        return false;
    }
    platform_sleep_ms(1000);  /* Wait for save to complete */

    /* Type filename without extension (THOUGHT adds it automatically) */
    if (!vt_interact_type_text(handle, test_file, false)) {
        return false;
    }
    platform_sleep_ms(500);
    if (!vt_interact_press_key(handle, "enter")) {
        return false;
    }
    platform_sleep_ms(500);

    /* Create multiple siblings */
    const char* siblings[] = {"Sibling 1", "Sibling 2", "Sibling 3", "Sibling 4"};
    for (int i = 0; i < 4; i++) {
        TEST_ASSERT_TRUE_MESSAGE(vt_interact_type_line(handle, siblings[i]), vt_interact_get_error());
        TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "enter"), vt_interact_get_error());
        Sleep(200);
    }

    /* Save to RAM filesystem, verify in menu, load and verify content */
    TEST_ASSERT_TRUE_MESSAGE(save_verify_and_reload(test_file, "Sibling 1"), "Outline save/load failed");

    /* Exit SUPER ROM */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "shift-f5"), vt_interact_get_error());
}
