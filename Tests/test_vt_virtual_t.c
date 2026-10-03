/**
 * @file test_vt_virtual_t.c
 * @brief Integration tests for VirtualT with model type 8201 and ROM loading
 */

#include "unity.h"
#include "vt_process.h"
#include "vt_socket.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#endif

/* Test fixtures */
static vt_process_handle_t process_handle = NULL;
static vt_socket_handle_t socket_handle = NULL;

/* Setup and teardown for Unity */
void setUp(void) {
    process_handle = NULL;
    socket_handle = NULL;
}

void tearDown(void) {
    /* Cleanup in proper order */
    if (socket_handle) {
        vt_socket_disconnect(socket_handle);
        vt_socket_destroy(socket_handle);
        socket_handle = NULL;
    }
    
    if (process_handle) {
        vt_process_terminate(process_handle);
        vt_process_destroy(process_handle);
        process_handle = NULL;
    }
}

/* Helper function to get VirtualT path from environment */
static const char* get_virtualt_path(void) {
    const char* path = getenv("VIRTUALT_PATH");
    TEST_ASSERT_NOT_NULL_MESSAGE(path, "VIRTUALT_PATH environment variable must be set");
    return path;
}

/* Helper function to get full ROM path */
static void get_rom_path(char* buffer, size_t size) {
#ifdef _WIN32
    /* Get the project root directory directly */
    const char* project_root = "C:/Users/John/projects/model_t/SuperROMPort";
    const char* rom_filename = "SUT/SUPNEC.bin";
    
    size_t root_len = strlen(project_root);
    if (root_len + 1 + strlen(rom_filename) + 1 < size) {
        strcpy_s(buffer, size, project_root);
        if (buffer[root_len - 1] != '/') {
            strncat_s(buffer, size, "/", size - strlen(buffer) - 1);
        }
        strncat_s(buffer, size, rom_filename, size - strlen(buffer) - 1);
    } else {
        /* Fallback to current directory method */
        DWORD len = GetCurrentDirectoryA((DWORD)size, buffer);
        const char* rom_filename = "SUT/SUPNEC.bin";
        size_t current_len = strlen(buffer);
        if (current_len > 0 && buffer[current_len - 1] != '/') {
            strncat_s(buffer, size, "/", size - current_len - 1);
        }
        strncat_s(buffer, size, rom_filename, size - strlen(buffer) - 1);
    }
#else
    getcwd(buffer, size);
    strncat_s(buffer, size, "/../../SUT/SUPNEC.bin", size - strlen(buffer) - 1);
#endif
}
/* Test: Basic process creation and destruction */
void test_process_create_destroy(void) {
    vt_process_handle_t handle = vt_process_create();
    TEST_ASSERT_NOT_NULL(handle);
    
    vt_process_destroy(handle);
}

/* Test: Launch VirtualT and verify it's running */
void test_launch_virtualt(void) {
    const char* vt_path = get_virtualt_path();
    
    process_handle = vt_process_create();
    TEST_ASSERT_NOT_NULL(process_handle);
    
    vt_process_config_t config = {
        .virtualt_path = vt_path,
        .port = 0,  /* Auto-assign port */
        .headless = true,
        .startup_timeout_ms = 30000
    };
    
    TEST_ASSERT_TRUE(vt_process_launch(process_handle, &config));
    TEST_ASSERT_TRUE(vt_process_is_running(process_handle));
    
    /* Wait for process to be ready */
    TEST_ASSERT_TRUE(vt_process_wait_for_ready(process_handle, 15000));
    
    /* Get the auto-assigned port */
    uint16_t port = vt_process_get_port(process_handle);
    TEST_ASSERT_NOT_EQUAL(0, port);
}

/* Test: Set model type to pc8201 (8201) */
void test_set_model_pc8201(void) {
    const char* vt_path = get_virtualt_path();
    
    process_handle = vt_process_create();
    TEST_ASSERT_NOT_NULL(process_handle);
    
    vt_process_config_t config = {
        .virtualt_path = vt_path,
        .port = 0,  /* Auto-assign port */
        .headless = true,
        .startup_timeout_ms = 30000
    };
    
    TEST_ASSERT_TRUE(vt_process_launch(process_handle, &config));
    TEST_ASSERT_TRUE(vt_process_wait_for_ready(process_handle, 15000));
    
    /* Connect socket to send commands using auto-assigned port */
    socket_handle = vt_socket_create("127.0.0.1", vt_process_get_port(process_handle));
    TEST_ASSERT_NOT_NULL(socket_handle);
    TEST_ASSERT_TRUE(vt_socket_connect(socket_handle));
    
    /* Set model to pc8201 */
    char response[256];
    TEST_ASSERT_TRUE(vt_socket_send_command(socket_handle, "model pc8201", response, sizeof(response)));
    
    /* Verify model is set by checking status */
    memset(response, 0, sizeof(response));
    TEST_ASSERT_TRUE(vt_socket_send_command(socket_handle, "status", response, sizeof(response)));
    TEST_ASSERT_TRUE(strstr(response, "pc8201") != NULL || strstr(response, "PC8201") != NULL);
    
    /* Cleanup */
    vt_socket_disconnect(socket_handle);
    vt_socket_destroy(socket_handle);
    socket_handle = NULL;
}

/* Test: Load option ROM SUT\SUPNEC.bin */
void test_load_option_rom(void) {
    const char* vt_path = get_virtualt_path();
    
    process_handle = vt_process_create();
    TEST_ASSERT_NOT_NULL(process_handle);
    
    vt_process_config_t config = {
        .virtualt_path = vt_path,
        .port = 0,  /* Auto-assign port */
        .headless = true,
        .startup_timeout_ms = 30000
    };
    
    TEST_ASSERT_TRUE(vt_process_launch(process_handle, &config));
    TEST_ASSERT_TRUE(vt_process_wait_for_ready(process_handle, 15000));
    
    /* Connect socket to send commands using auto-assigned port */
    socket_handle = vt_socket_create("127.0.0.1", vt_process_get_port(process_handle));
    TEST_ASSERT_NOT_NULL(socket_handle);
    TEST_ASSERT_TRUE(vt_socket_connect(socket_handle));
    
    /* Get full path to ROM file */
    char rom_path[512];
    get_rom_path(rom_path, sizeof(rom_path));
    
    /* Load ROM */
    char response[512];
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "optrom %s", rom_path);
    TEST_ASSERT_TRUE(vt_socket_send_command(socket_handle, cmd, response, sizeof(response)));
    
    /* ROM loading should return Ok */
    TEST_ASSERT_TRUE(strstr(response, "Ok") != NULL);
    
    /* Cleanup */
    vt_socket_disconnect(socket_handle);
    vt_socket_destroy(socket_handle);
    socket_handle = NULL;
}
/* Test: Complete workflow - launch, set model, load ROM, cleanup */
void test_complete_workflow(void) {
    const char* vt_path = get_virtualt_path();
    
    /* Step 1: Launch VirtualT */
    process_handle = vt_process_create();
    TEST_ASSERT_NOT_NULL(process_handle);
    
    vt_process_config_t config = {
        .virtualt_path = vt_path,
        .port = 0,  /* Auto-assign port */
        .headless = true,
        .startup_timeout_ms = 30000
    };
    
    TEST_ASSERT_TRUE(vt_process_launch(process_handle, &config));
    TEST_ASSERT_TRUE(vt_process_is_running(process_handle));
    TEST_ASSERT_TRUE(vt_process_wait_for_ready(process_handle, 15000));
    
    /* Step 2: Connect and set model to pc8201 */
    socket_handle = vt_socket_create("127.0.0.1", vt_process_get_port(process_handle));
    TEST_ASSERT_NOT_NULL(socket_handle);
    TEST_ASSERT_TRUE(vt_socket_connect(socket_handle));
    
    char response[256];
    TEST_ASSERT_TRUE(vt_socket_send_command(socket_handle, "model pc8201", response, sizeof(response)));
    TEST_ASSERT_TRUE(strstr(response, "Ok") != NULL);
    
    /* Verify model in status */
    memset(response, 0, sizeof(response));
    TEST_ASSERT_TRUE(vt_socket_send_command(socket_handle, "status", response, sizeof(response)));
    TEST_ASSERT_TRUE(strstr(response, "pc8201") != NULL || strstr(response, "PC8201") != NULL);
    
    /* Step 3: Load ROM */
    char rom_path2[512];
    get_rom_path(rom_path2, sizeof(rom_path2));
    
    char cmd2[1024];
    snprintf(cmd2, sizeof(cmd2), "optrom %s", rom_path2);
    memset(response, 0, sizeof(response));
    TEST_ASSERT_TRUE(vt_socket_send_command(socket_handle, cmd2, response, sizeof(response)));
    TEST_ASSERT_TRUE(strstr(response, "Ok") != NULL);
    
    /* Step 4: Cleanup */
    vt_socket_disconnect(socket_handle);
    vt_socket_destroy(socket_handle);
    socket_handle = NULL;
    
    vt_process_terminate(process_handle);
    TEST_ASSERT_FALSE(vt_process_is_running(process_handle));
    
    vt_process_destroy(process_handle);
    process_handle = NULL;
}

int main(void) {
    const char* error;
    
    UNITY_BEGIN();
    
    /* Run all tests */
    RUN_TEST(test_process_create_destroy);
    RUN_TEST(test_launch_virtualt);
    RUN_TEST(test_set_model_pc8201);
    RUN_TEST(test_load_option_rom);
    RUN_TEST(test_complete_workflow);
    
    /* Print error info if tests failed */
    error = vt_process_get_error();
    if (error && error[0] != '\0') {
        printf("Error: %s\n", error);
    }
    
    return UNITY_END();
}