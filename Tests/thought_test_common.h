/**
 * @file thought_test_common.h
 * @brief Common utilities for THOUGHT tests
 *
 * These utilities help test THOUGHT outline functionality.
 * Files saved in THOUGHT appear in the RAM filesystem (main menu),
 * not cassette. We verify operation by checking the main menu
 * for the saved file and via screen memory when loading.
 */

#ifndef THOUGHT_TEST_COMMON_H
#define THOUGHT_TEST_COMMON_H

#include "vt_interact.h"
#include <string.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#endif

/* RAM filesystem directory starts at 0xF84F in VirtualT */
#define VT_RAM_DIR_ADDRESS  0xF84F
#define VT_RAM_DIR_SIZE     88

/* Verify outline content by reading screen memory at 0xFD00 */
static bool verify_outline_content(vt_interact_handle_t handle, const char* expected_text) {
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
static bool verify_ram_file_exists(vt_interact_handle_t handle, const char* filename) {
    char dir_buffer[VT_RAM_DIR_SIZE];
    bool success = vt_interact_read_memory(handle, VT_RAM_DIR_ADDRESS, VT_RAM_DIR_SIZE, dir_buffer, sizeof(dir_buffer));

    if (!success) {
        printf("Failed to read RAM directory\n");
        return false;
    }

    /* Search for filename in directory (8-char filename + 1-char extension format) */
    size_t filename_len = strlen(filename);
    for (int i = 0; i <= VT_RAM_DIR_SIZE - 8; i++) {
        bool match = true;
        for (int j = 0; j < 8; j++) {
            char dir_char = dir_buffer[i + j];
            char file_char = (j < filename_len) ? filename[j] : ' ';

            /* RAM directory pads with spaces, so trailing spaces in filename match any dir space */
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

/* Enter THOUGHT from SUPER ROM menu */
static bool enter_thought(vt_interact_handle_t handle) {
    /* Navigate to SUPER ROM menu (row 3) */
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

    return true;
}

/* Save outline to RAM filesystem and exit to main menu */
static bool save_to_ram_filesystem(vt_interact_handle_t handle, const char* filename) {
    /* Save via F3 then Enter (saves to RAM filesystem by default) */
    if (!vt_interact_press_key(handle, "f3")) {
        return false;
    }
    Sleep(300);
    if (!vt_interact_press_key(handle, "enter")) {
        return false;
    }
    Sleep(500);

    /* Exit THOUGHT back to SUPER ROM menu (F8 twice) */
    if (!vt_interact_press_key(handle, "f8")) {
        return false;
    }
    Sleep(300);
    if (!vt_interact_press_key(handle, "f8")) {
        return false;
    }
    Sleep(300);

    /* Exit SUPER ROM to main NEC menu (F8) */
    if (!vt_interact_press_key(handle, "f8")) {
        return false;
    }
    Sleep(300);

    return true;
}

/* Load outline from RAM filesystem and return to THOUGHT */
static bool load_from_ram_filesystem(vt_interact_handle_t handle, const char* filename) {
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

    return true;
}

/* Save to RAM filesystem, re-enter THOUGHT, and verify content */
static bool save_and_verify_thought_outline(vt_interact_handle_t handle, const char* test_file, const char* expected_content) {
    /* Save to RAM filesystem and exit to main menu */
    if (!save_to_ram_filesystem(handle, test_file)) {
        return false;
    }

    /* Verify file exists in RAM filesystem */
    if (!verify_ram_file_exists(handle, test_file)) {
        printf("File '%s' not found in RAM filesystem after save\n", test_file);
        return false;
    }

    /* Re-enter THOUGHT and load the file */
    if (!load_from_ram_filesystem(handle, test_file)) {
        return false;
    }

    /* Verify content via screen memory */
    if (!verify_outline_content(handle, expected_content)) {
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

#endif /* THOUGHT_TEST_COMMON_H */
