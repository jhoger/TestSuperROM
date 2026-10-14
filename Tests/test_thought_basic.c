/**
 * @file test_thought_basic.c
 * @brief Basic THOUGHT test to verify keystrokes work
 */

#include "unity.h"
#include "platform_compat.h"
#include "vt_interact.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#endif

/* Shared handle for test functions */
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
    platform_sleep_ms(500);
}

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

/* Print a screen dump for debugging */
static void print_screen_dump(vt_interact_handle_t handle) {
    char response[4096];
    if (vt_interact_send_command(handle, "sd", response, sizeof(response))) {
        printf("\n=== SCREEN DUMP ===\n%s\n=== END SCREEN DUMP ===\n", response);
    } else {
        printf("Failed to get screen dump\n");
    }
    fflush(stdout);
}

static bool verify_file_in_ram(const char* filename) {
    uint8_t dir_buffer[88];
    bool success = vt_interact_read_memory_raw(handle, 0xF84F, 88, dir_buffer, sizeof(dir_buffer));

    printf("read_memory_raw returned: %s\n", success ? "true" : "false");

    if (!success) {
        printf("Failed to read RAM directory\n");
        return false;
    }

    printf("RAM Directory contents (hex):\n");
    fflush(stdout);
    for (int i = 0; i < 88; i++) {
        printf("%02X ", (unsigned char)dir_buffer[i]);
        fflush(stdout);
        if ((i + 1) % 11 == 0) {
            printf("\n");
            fflush(stdout);
        }
    }
    printf("\n");
    fflush(stdout);

    const int entries = 88 / 11;
    const int filename_offset = 3;

    for (int e = 0; e < entries; e++) {
        unsigned char* entry = (unsigned char*)&dir_buffer[e * 11];
        unsigned char dir_flag = entry[0];
        if (dir_flag == 0) {
            printf("Entry %d: EMPTY\n", e);
            continue;
        }

        char entry_filename[9];
        memcpy(entry_filename, &entry[filename_offset], 8);
        entry_filename[8] = '\0';

        /* Debug: show raw bytes before stripping */
        size_t entry_len = strlen(entry_filename);
        printf("DEBUG: Entry %d before stripping: len=%zu, filename='%s'\n", e, entry_len, entry_filename);
        fflush(stdout);

        int len = strlen(entry_filename);
        while (len > 0 && entry_filename[len-1] == ' ') {
            entry_filename[len-1] = '\0';
            len--;
        }

        printf("DEBUG: About to print Entry %d main info\n", e);
        fflush(stdout);
        printf("Entry %d: flag=0x%02X, filename='%s'\n", e, dir_flag, entry_filename);
        fflush(stdout);

        printf("DEBUG: Comparing '%s' with '%s'\n", entry_filename, filename);
        fflush(stdout);
        if (strcmp(entry_filename, filename) == 0) {
            printf("DEBUG: Match found! Now returning...\n");
            fflush(stdout);
            /* Try to allocate to verify heap is intact */
            char* test = (char*)malloc(10);
            if (test) {
                strcpy(test, "test");
                printf("DEBUG: Heap test passed\n");
                fflush(stdout);
                free(test);
            } else {
                printf("DEBUG: Heap test failed - malloc returned NULL\n");
                fflush(stdout);
            }
            printf("DEBUG: Now doing return true\n");
            fflush(stdout);
            return true;
        }
    }

    return false;
}

void test_thought_basic(void) {
    const char* vt_path = get_virtualt_path();
    char screen[512];

    printf("=== Test: Basic THOUGHT ===\n");
    fflush(stdout);

    vt_interact_handle_t handle_local = vt_interact_create();
    TEST_ASSERT_NOT_NULL(handle_local);
    handle = handle_local;

    vt_interact_config_t config = {
        .virtualt_path = vt_path,
        .port = 0,
        .headless = true,
        .startup_timeout_ms = 30000
    };

    printf("Launching...\n");
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_launch(handle, &config), vt_interact_get_error());
    printf("Waiting for ready...\n");
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_wait_for_ready(handle, 15000), vt_interact_get_error());
    printf("Connecting...\n");
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_connect(handle), vt_interact_get_error());

    printf("Cold boot to format RAM filesystem...\n");
    char response[256];
    bool cold_boot_ok = vt_interact_send_command(handle, "cold_boot", response, sizeof(response));
    TEST_ASSERT_TRUE_MESSAGE(cold_boot_ok, "Cold boot failed");
    platform_sleep_ms(1000);

    printf("Setting model...\n");
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_set_model(handle, "pc8201"), vt_interact_get_error());

    printf("Loading ROM...\n");
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_load_rom(handle), vt_interact_get_error());
    platform_sleep_ms(500);

    printf("Launching SUPER ROM from menu...\n");
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_launch_basic_from_menu(handle), vt_interact_get_error());
    platform_sleep_ms(Sleep(300));

    printf("Loading SUPER ROM via EXEC...\n");
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_type_line(handle, "EXEC 62394"), vt_interact_get_error());
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "enter"), vt_interact_get_error());
    platform_sleep_ms(500);

    /* Exit to shell menu */
    printf("Exiting to shell menu...\n");
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_exit_to_menu(handle), vt_interact_get_error());
    platform_sleep_ms(Sleep(1500));
    print_screen_dump(handle);

    /* From shell menu, navigate to BASIC (row 0) */
    printf("Navigating to BASIC...\n");
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_menu_navigate(handle, 0, 0), vt_interact_get_error());
    platform_sleep_ms(Sleep(300));
    print_screen_dump(handle);

    /* Enter BASIC */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "enter"), vt_interact_get_error());
    platform_sleep_ms(Sleep(300));

    /* Exit BASIC to get to shell menu */
    printf("Exiting BASIC...\n");
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "shift-f5"), vt_interact_get_error());
    platform_sleep_ms(1000);
    print_screen_dump(handle);

    /* SUPER ROM was installed earlier with EXEC 62394.
       Now we need to launch SuperROM from the shell menu. */
    printf("Launching SuperROM via BASIC...\n");

    /* Navigate to BASIC (row 0) */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_menu_navigate(handle, 0, 0), vt_interact_get_error());
    platform_sleep_ms(Sleep(300));
    print_screen_dump(handle);

    /* Enter BASIC */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "enter"), vt_interact_get_error());
    platform_sleep_ms(500);
    print_screen_dump(handle);

    /* Load SuperROM from BASIC */
    printf("Loading SuperROM via EXEC 62394...\n");
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_type_line(handle, "EXEC 62394"), vt_interact_get_error());
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "enter"), vt_interact_get_error());
    platform_sleep_ms(Sleep(3000));  /* Wait for SuperROM to fully initialize */
    print_screen_dump(handle);

    /* From SuperROM, press F3 to launch THOUGHT directly */
    printf("Pressing F3 to launch THOUGHT...\n");
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "f3"), vt_interact_get_error());
    platform_sleep_ms(Sleep(2000));
    print_screen_dump(handle);

    /* Verify THOUGHT is running */
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_get_screen_dump(handle, screen, sizeof(screen)), "Failed to get screen dump");
    printf("Current screen: %.100s...\n", screen);

    if (strstr(screen, "filename") == NULL && strstr(screen, "?") == NULL) {
        printf("WARNING: THOUGHT may not be running correctly\n");
        print_screen_dump(handle);
    }

    /* Type filename after THOUGHT prompt appears */
    printf("Typing filename 'TEST'...\n");
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_type_text(handle, "TEST", true), vt_interact_get_error());
    platform_sleep_ms(Sleep(1500));
    print_screen_dump(handle);

    /* THOUGHT should now be running with the new file */
    printf("THOUGHT should be running with TEST file...\n");
    platform_sleep_ms(1000);
    print_screen_dump(handle);

    /* Exit THOUGHT */
    printf("Exiting THOUGHT...\n");
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "shift-f5"), vt_interact_get_error());
    platform_sleep_ms(500);
    print_screen_dump(handle);

    /* Exit to main NEC menu */
    printf("Exiting to main menu...\n");
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "shift-f5"), vt_interact_get_error());
    platform_sleep_ms(500);
    print_screen_dump(handle);

    /* Exit to THOUGHT menu */
    printf("Exiting to THOUGHT menu...\n");
    TEST_ASSERT_TRUE_MESSAGE(vt_interact_press_key(handle, "shift-f5"), vt_interact_get_error());
    platform_sleep_ms(500);
    print_screen_dump(handle);

    /* Verify file was created */
    printf("Verifying file in RAM...\n");
    print_screen_dump(handle);

    /* Read and check the directory directly */
    printf("DEBUG: Before read_memory_raw, handle=%p\n", (void*)handle);
    fflush(stdout);
    uint8_t dir_buffer[88];
    bool success = vt_interact_read_memory_raw(handle, 0xF84F, 88, dir_buffer, sizeof(dir_buffer));
    printf("DEBUG: After read_memory_raw, success=%d\n", success);
    fflush(stdout);
    printf("read_memory_raw returned: %s\n", success ? "true" : "false");
    fflush(stdout);

    printf("DEBUG: About to enter if block\n");
    fflush(stdout);
    if (success) {
        printf("DEBUG: Inside if block, about to print hex\n");
        fflush(stdout);
        printf("RAM Directory contents (hex):\n");
        printf("DEBUG: About to start hex loop, dir_buffer address=%p\n", (void*)dir_buffer);
        fflush(stdout);
        for (int i = 0; i < 88; i++) {
            printf("%02X ", (unsigned char)dir_buffer[i]);
            if ((i + 1) % 11 == 0) printf("\n");
            if (i == 87) {
                printf("DEBUG: Hex loop completed\n");
                fflush(stdout);
            }
        }
        printf("\n");
        printf("DEBUG: After hex loop, about to enter for loop\n");
        fflush(stdout);

        bool file_found = false;
        const int entries = 88 / 11;
        const int filename_offset = 3;

        printf("DEBUG: entries=%d, filename_offset=%d\n", entries, filename_offset);
        fflush(stdout);
        for (int e = 0; e < entries; e++) {
            printf("DEBUG: For loop iteration %d\n", e);
            fflush(stdout);
            unsigned char* entry = (unsigned char*)&dir_buffer[e * 11];
            unsigned char dir_flag = entry[0];
            if (dir_flag == 0) {
                printf("Entry %d: EMPTY\n", e);
                continue;
            }

            char entry_filename[9];
            memcpy(entry_filename, &entry[filename_offset], 8);
            entry_filename[8] = '\0';

            /* Strip trailing spaces */
            int len = strlen(entry_filename);
            while (len > 0 && entry_filename[len-1] == ' ') {
                entry_filename[len-1] = '\0';
                len--;
            }

            printf("DEBUG: Before entry printf, e=%d, entry_filename='%s'\n", e, entry_filename);
            fflush(stdout);
            char buf[100];
            snprintf(buf, sizeof(buf), "Entry %d: flag=0x%02X, filename='%s'\n", e, dir_flag, entry_filename);
            printf("%s", buf);
            fflush(stdout);

            printf("DEBUG: About to strcmp, entry_filename='%s'\n", entry_filename);
            fflush(stdout);
            const char* expected = "TEST  CT";
            printf("DEBUG: expected='%s'\n", expected);
            fflush(stdout);
            size_t entry_len = strlen(entry_filename);
            size_t expected_len = strlen(expected);
            printf("DEBUG: entry_len=%zu, expected_len=%zu\n", entry_len, expected_len);
            fflush(stdout);
            /* Compare with expected filename (including extension as stored) */
            int match = (entry_len == expected_len && memcmp(entry_filename, expected, entry_len) == 0);
            printf("DEBUG: memcmp result: %d\n", match);
            fflush(stdout);
            if (match) {
                printf("DEBUG: Match! About to print found message\n");
                fflush(stdout);
                printf("File 'TEST  CT' found!\n");
                fflush(stdout);
                file_found = true;
                printf("DEBUG: About to break\n");
                fflush(stdout);
                break;
            }
            printf("DEBUG: Done with iteration %d\n", e);
            fflush(stdout);
        }
        printf("DEBUG: Loop exited, file_found=%d\n", file_found);
        fflush(stdout);

        printf("DEBUG: About to call TEST_ASSERT_TRUE_MESSAGE\n");
        fflush(stdout);
        TEST_ASSERT_TRUE_MESSAGE(file_found, "TEST not found in RAM filesystem");
        printf("DEBUG: After TEST_ASSERT_TRUE_MESSAGE\n");
        fflush(stdout);
    } else {
        TEST_FAIL_MESSAGE("Failed to read RAM directory");
    }

    printf("DEBUG: About to print Terminating\n");
    fflush(stdout);
    printf("Terminating...\n");
    fflush(stdout);
    vt_interact_terminate(handle);
    printf("DEBUG: After vt_interact_terminate\n");
    fflush(stdout);
    vt_interact_destroy(handle);
    printf("DEBUG: After vt_interact_destroy\n");
    fflush(stdout);
    handle = NULL;  /* Prevent tearDown() from double-freeing */
    printf("Test passed!\n");
    fflush(stdout);
    printf("DEBUG: After Test passed!\n");
    fflush(stdout);
}

int main(int argc, char** argv) {
    UnityParseOptions(argc, argv);
    UNITY_BEGIN();
    RUN_TEST(test_thought_basic);
    return UNITY_END();
}
