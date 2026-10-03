/**
 * @file vt_process.h
 * @brief VirtualT Process Management Library
 *
 * Provides process lifecycle management for VirtualT emulator.
 * Integrates with vt_socket for seamless connection management.
 */

#ifndef VT_PROCESS_H
#define VT_PROCESS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "vt_socket.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Constants */
#define VT_PROCESS_SUCCESS           0
#define VT_PROCESS_ERROR            -1
#define VT_PROCESS_TIMEOUT          -2
#define VT_PROCESS_ALREADY_STARTED  -3
#define VT_PROCESS_NOT_STARTED      -4
#define VT_PROCESS_CONNECTION_FAIL  -5

/* Platform-specific includes */
#ifdef _WIN32
    #include <windows.h>
    #define VT_PROCESS_PID HANDLE
#else
    #include <sys/types.h>
    #define VT_PROCESS_PID pid_t
#endif

/* Type Definitions */
typedef struct vt_process_t* vt_process_handle_t;

/* Process status structure */
typedef struct {
    bool is_running;
    bool is_connected;
    uint16_t port;
    VT_PROCESS_PID pid;
} vt_process_status_t;

/* Process configuration */
typedef struct {
    const char* virtualt_path;
    uint16_t port;             /* Socket port (0 = auto-assign) */
    const char* rom_path;
    bool headless;
    uint32_t startup_timeout_ms;
} vt_process_config_t;

/* Convenience macro for auto-assigning port */
#define VT_PROCESS_PORT_AUTO 0

/* Event callback type */
typedef void (*vt_process_callback_t)(const char* event, void* user_data);

/* ============================================================================
 * Initialization and Cleanup
 * ========================================================================= */

vt_process_handle_t vt_process_create(void);
void vt_process_destroy(vt_process_handle_t handle);

/* ============================================================================
 * Process Management
 * ========================================================================= */

bool vt_process_launch(vt_process_handle_t handle,
                       const vt_process_config_t* config);
bool vt_process_wait_for_ready(vt_process_handle_t handle,
                               uint32_t timeout_ms);
bool vt_process_connect(vt_process_handle_t handle,
                        vt_socket_handle_t socket_handle);
void vt_process_disconnect(vt_process_handle_t handle);
bool vt_process_terminate(vt_process_handle_t handle);

/* ============================================================================
 * Helper Functions
 * ========================================================================= */

/* Create a socket handle connected to this process's port */
vt_socket_handle_t vt_process_create_socket(vt_process_handle_t handle,
                                            const char* host);

/* ============================================================================
 * Status Functions
 * ========================================================================= */

bool vt_process_get_status(vt_process_handle_t handle,
                           vt_process_status_t* status);
uint16_t vt_process_get_port(vt_process_handle_t handle);
bool vt_process_is_running(vt_process_handle_t handle);
bool vt_process_is_connected(vt_process_handle_t handle);

/* ============================================================================
 * Error Handling
 * ========================================================================= */

const char* vt_process_get_error(void);
void vt_process_set_error(const char* msg);

/* ============================================================================
 * Event Callbacks
 * ========================================================================= */

bool vt_process_set_callback(vt_process_handle_t handle,
                             vt_process_callback_t callback,
                             void* user_data);

#ifdef __cplusplus
}
#endif

#endif /* VT_PROCESS_H */