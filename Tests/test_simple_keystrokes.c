/**
 * @file test_simple_keystrokes.c
 * @brief Simple test for vt_interact_send_keystrokes
 */

#define _CRT_SECURE_NO_WARNINGS

#include "vt_interact.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#endif

static const char* get_virtualt_path(void) {
    const char* path = getenv("VIRTUALT_PATH");
    if (!path) {
        #ifdef _WIN32
        _putenv("VIRTUALT_PATH=C:/Users/John/tools/VirtualT/VirtualT.exe");
        path = getenv("VIRTUALT_PATH");
        #endif
    }
    return path;
}

int main(void) {
    /* Disable stdout buffering for immediate output */
    setvbuf(stdout, NULL, _IONBF, 0);

    printf("Starting test...\n");
    fflush(stdout);

    vt_interact_handle_t handle = vt_interact_create();
    if (!handle) {
        printf("Failed to create handle\n");
        return 1;
    }
    printf("Handle created\n");

    const char* vt_path = get_virtualt_path();
    printf("Using VirtualT path: %s\n", vt_path);

    vt_interact_config_t config = {
        .virtualt_path = vt_path,
        .port = 0,
        .headless = true,
        .startup_timeout_ms = 30000
    };

    printf("Launching VirtualT...\n");
    if (!vt_interact_launch(handle, &config)) {
        printf("Failed to launch: %s\n", vt_interact_get_error());
        return 1;
    }
    printf("Launch successful\n");

    printf("Waiting for ready...\n");
    if (!vt_interact_wait_for_ready(handle, 30000)) {
        printf("Failed to wait for ready: %s\n", vt_interact_get_error());
        return 1;
    }
    printf("Ready\n");

    printf("Connecting...\n");
    if (!vt_interact_connect(handle)) {
        printf("Failed to connect: %s\n", vt_interact_get_error());
        return 1;
    }
    printf("Connected\n");

    printf("Setting model...\n");
    if (!vt_interact_set_model(handle, "pc8201")) {
        printf("Failed to set model: %s\n", vt_interact_get_error());
        return 1;
    }
    printf("Model set\n");

    printf("Loading ROM...\n");
    if (!vt_interact_load_rom(handle)) {
        printf("Failed to load ROM: %s\n", vt_interact_get_error());
        return 1;
    }
    printf("ROM loaded\n");

    printf("Launching BASIC from menu...\n");
    if (!vt_interact_launch_basic_from_menu(handle)) {
        printf("Failed to launch BASIC: %s\n", vt_interact_get_error());
        return 1;
    }
    printf("BASIC launched\n");

    Sleep(500);

    printf("Sending keystrokes...\n");
    const char* keys[] = {"T", "E", "S", "T"};
    int error_code = 0;
    bool result = vt_interact_send_keystrokes(handle, keys, 4, 2000, "TEST", true, &error_code);

    printf("Keystrokes result: %d, error_code: %d, error: %s\n",
           result, error_code, vt_interact_get_error());

    printf("Terminating...\n");
    vt_interact_terminate(handle);
    printf("Terminated\n");

    vt_interact_destroy(handle);
    printf("Destroyed handle\n");

    printf("Test complete!\n");
    return 0;
}
