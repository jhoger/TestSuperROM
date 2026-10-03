/**
 * @file vt_interact.h
 * @brief High-level interaction library for VirtualT emulator
 * 
 * This library provides higher-level functions for interacting with VirtualT
 * emulated computers, specifically the NEC PC-8201A. It builds on top of
 * vt_socket and vt_process to provide convenient operations for:
 * - Launching the PC-8201A
 * - Navigating menus
 * - Launching programs (especially BASIC)
 * - Entering and executing BASIC programs
 */

#ifndef VT_INTERACT_H
#define VT_INTERACT_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "vt_process.h"
#include "vt_socket.h"

#ifdef __cplusplus
extern "C" {
#endif

/* PC-8201A specific constants */
#define VT_PC8201_LCD_ROWS      8
#define VT_PC8201_LCD_COLS      40
#define VT_PC8201_ROM_PATH      "SUT/SUPNEC.bin"

/* Return codes */
#define VT_INTERACT_SUCCESS         0
#define VT_INTERACT_ERROR          -1
#define VT_INTERACT_TIMEOUT        -2
#define VT_INTERACT_NOT_STARTED    -3
#define VT_INTERACT_CONNECTION_ERR -4

/* PC-8201A memory addresses for keyboard input */
/* PC-8201A keyboard buffer at 0x40C-0x41F */
#define VT_PC8201_KBD_BUFFER      0x40C
#define VT_PC8201_KBD_HEAD        0x408
#define VT_PC8201_KBD_TAIL        0x40A

/* Type Definitions */
typedef struct vt_interact_t* vt_interact_handle_t;

/* PC-8201A configuration */
typedef struct {
    const char* virtualt_path;
    uint16_t port;
    bool headless;
    uint32_t startup_timeout_ms;
} vt_interact_config_t;

/* PC-8201A status */
typedef struct {
    bool is_running;
    bool is_connected;
    bool in_menu;
    bool in_basic;
    uint16_t pc;
    uint16_t sp;
} vt_interact_status_t;

/* ============================================================================
 * Initialization and Cleanup
 * ========================================================================= */

vt_interact_handle_t vt_interact_create(void);
void vt_interact_destroy(vt_interact_handle_t handle);

/* ============================================================================
 * Process Management
 * ========================================================================= */

bool vt_interact_launch(vt_interact_handle_t handle,
                       const vt_interact_config_t* config);
bool vt_interact_wait_for_ready(vt_interact_handle_t handle,
                               uint32_t timeout_ms);
bool vt_interact_terminate(vt_interact_handle_t handle);

/* ============================================================================
 * Socket Connection
 * ========================================================================= */

bool vt_interact_connect(vt_interact_handle_t handle);
void vt_interact_disconnect(vt_interact_handle_t handle);
bool vt_interact_is_connected(vt_interact_handle_t handle);

/* ============================================================================
 * System Control
 * ========================================================================= */

bool vt_interact_set_model(vt_interact_handle_t handle, const char* model);
bool vt_interact_get_status(vt_interact_handle_t handle, vt_interact_status_t* status);
bool vt_interact_halt_cpu(vt_interact_handle_t handle);
bool vt_interact_run_cpu(vt_interact_handle_t handle);

/* ============================================================================
 * LCD Operations
 * ========================================================================= */

bool vt_interact_lcd_clear(vt_interact_handle_t handle);
bool vt_interact_lcd_write(vt_interact_handle_t handle, uint8_t row, uint8_t col, const char* data);
bool vt_interact_lcd_get(vt_interact_handle_t handle, char lcd_state[VT_PC8201_LCD_ROWS][VT_PC8201_LCD_COLS]);

/* ============================================================================
 * PC-8201A Specific Operations
 * ========================================================================= */

bool vt_interact_load_rom(vt_interact_handle_t handle);
bool vt_interact_launch_basic(vt_interact_handle_t handle);
bool vt_interact_type_line(vt_interact_handle_t handle, const char* line);
bool vt_interact_execute_program(vt_interact_handle_t handle);

/* ============================================================================
 * Helper Functions
 * ========================================================================= */

uint16_t vt_interact_get_port(vt_interact_handle_t handle);
bool vt_interact_is_running(vt_interact_handle_t handle);
const char* vt_interact_get_error(void);

#ifdef __cplusplus
}
#endif

#endif /* VT_INTERACT_H */