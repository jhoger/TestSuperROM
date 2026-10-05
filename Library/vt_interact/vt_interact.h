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
#define VT_INTERACT_MATCH_ERR      -5

/* PC-8201A cursor position registers */
/* Per web search: CSRY=F3E5h, CSRX=F3E6h, LCDCSY=F3ECh, LCDCSX=F3EDh */
#define VT_PC8201_CURSOR_X        0xF3E6  /* CSRX - Current cursor X (1-40) */
#define VT_PC8201_CURSOR_Y        0xF3E5  /* CSRY - Current cursor Y (1-8) */

/* PC-8201A LCD VRAM start address */
#define VT_PC8201_LCD_VRAM        0xFD00

/* PC-8201A memory addresses for keyboard input */
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
 * LCD Monitoring
 * ========================================================================= */

/**
 * Enable/disable LCD monitoring via the lcd_mon command
 * @param handle - vt_interact handle
 * @param enable - true to enable monitoring, false to disable
 * @return true on success
 */
bool vt_interact_lcd_monitor_enable(vt_interact_handle_t handle, bool enable);

/**
 * Get the most recent LCD event
 * @param handle - vt_interact handle
 * @param row - output: row number (0-7)
 * @param col - output: column number (0-39)
 * @param data - output: character data written to LCD
 * @param data_size - size of data buffer
 * @return true if an LCD event was received, false otherwise
 */
bool vt_interact_get_lcd_event(vt_interact_handle_t handle, uint8_t* row, uint8_t* col, char* data, size_t data_size);

/* ============================================================================
 * PC-8201A Specific Operations
 * ========================================================================= */

bool vt_interact_load_rom(vt_interact_handle_t handle);
bool vt_interact_launch_basic(vt_interact_handle_t handle);
bool vt_interact_type_line(vt_interact_handle_t handle, const char* line);
bool vt_interact_execute_program(vt_interact_handle_t handle);

/* ============================================================================
 * Menu Navigation Functions
 * ========================================================================= */

/**
 * Navigate to an entry in the PC-8201A menu using arrow keys
 * @param handle - vt_interact handle
 * @param row - Target row (0-7 for 8-row screen)
 * @param current_row - Current row position
 * @return true on success
 */
bool vt_interact_menu_navigate(vt_interact_handle_t handle, uint8_t row, uint8_t current_row);

/**
 * Launch the BASIC interpreter by navigating the menu
 * @param handle - vt_interact handle
 * @return true on success
 */
bool vt_interact_launch_basic_from_menu(vt_interact_handle_t handle);

/**
 * Type a complete BASIC program (one line at a time)
 * @param handle - vt_interact handle
 * @param lines - Array of BASIC lines
 * @param count - Number of lines
 * @return true on success
 */
bool vt_interact_type_program(vt_interact_handle_t handle, const char* lines[], int count);

/**
 * Exit back to the main menu (F8 key)
 * @param handle - vt_interact handle
 * @return true on success
 */
bool vt_interact_exit_to_menu(vt_interact_handle_t handle);

/**
 * Press a specific key
 * @param handle - vt_interact handle
 * @param key_name - Key name (e.g., "enter", "left", "right", "up", "down", "f8")
 * @return true on success
 */
bool vt_interact_press_key(vt_interact_handle_t handle, const char* key_name);

/**
 * Type text with optional line ending
 * @param handle - vt_interact handle
 * @param text - Text to type
 * @param newline - true to append ENTER
 * @return true on success
 */
bool vt_interact_type_text(vt_interact_handle_t handle, const char* text, bool newline);

/**
 * Send a raw command to VirtualT socket (for testing/file operations)
 * @param handle - vt_interact handle
 * @param command - Command string to send
 * @param response - Buffer to receive response
 * @param response_size - Size of response buffer
 * @return true if command was sent and response received
 */
bool vt_interact_send_command(vt_interact_handle_t handle, const char* command, char* response, size_t response_size);

/**
 * Read memory from the emulated system and verify it contains expected content
 * @param handle - vt_interact handle
 * @param address - Memory address to read
 * @param length - Number of bytes to read
 * @param expected_content - String to search for
 * @return true if content was found
 */
bool vt_interact_verify_mem_contains(vt_interact_handle_t handle, uint16_t address, size_t length, const char* expected_content);

/**
 * Send keystrokes and verify expected display output
 * @param handle - vt_interact handle
 * @param keystrokes - Array of keystroke strings (text or key names like "enter", "f8")
 * @param keystroke_count - Number of keystrokes in the array
 * @param timeout_ms - Timeout in milliseconds for command execution
 * @param expected_output - Expected text displayed on screen after keystrokes
 * @param reset - If true, assume screen resets (cursor goes to 0,0) as side-effect
 * @param error_code - Output: error code on failure (optional, can be NULL)
 * @return true on success, false on failure with error_code set
 */
bool vt_interact_send_keystrokes(vt_interact_handle_t handle,
                                const char* keystrokes[],
                                int keystroke_count,
                                uint32_t timeout_ms,
                                const char* expected_output,
                                bool reset,
                                int* error_code);

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
