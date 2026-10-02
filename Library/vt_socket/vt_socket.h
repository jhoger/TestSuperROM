/**
 * @file vt_socket.h
 * @brief VirtualT Socket Interface Library
 *
 * Provides both synchronous and asynchronous socket communication with VirtualT
 * for testing and automation purposes.
 */

#ifndef VT_SOCKET_H
#define VT_SOCKET_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Constants */
#define VT_DEFAULT_PORT       6166
#define VT_MAX_BUFFER_SIZE    4096
#define VT_MAX_COMMAND_LEN    256
#define VT_SOCKET_TIMEOUT_MS  5000
#define VT_LCD_MAX_ROWS       4
#define VT_LCD_MAX_COLS       20

/* Type Definitions */
typedef struct vt_socket_t* vt_socket_handle_t;
typedef struct vt_event_monitor_t* vt_event_monitor_handle_t;
typedef void (*vt_event_callback_t)(const char* event_name, 
                                     const char* event_data,
                                     void* user_data);
typedef struct {
    uint8_t row;
    uint8_t col;
    char data[VT_LCD_MAX_COLS];
    size_t length;
} vt_lcd_update_t;
typedef struct {
    const char* model;
    bool is_running;
} vt_cpu_status_t;
typedef struct {
    uint8_t a, b, c, d, e, h, l;
    uint16_t bc, de, hl, sp, pc;
} vt_registers_t;

/* ============================================================================
 * Initialization and Cleanup
 * ========================================================================= */

bool vt_socket_init(void);
void vt_socket_cleanup(void);
vt_socket_handle_t vt_socket_create(const char* host, uint16_t port);
void vt_socket_destroy(vt_socket_handle_t handle);

/* ============================================================================
 * Synchronous Operations
 * ========================================================================= */

bool vt_socket_connect(vt_socket_handle_t handle);
void vt_socket_disconnect(vt_socket_handle_t handle);
bool vt_socket_send_command(vt_socket_handle_t handle,
                            const char* command,
                            char* response,
                            size_t response_size);
bool vt_socket_send_command_fmt(vt_socket_handle_t handle,
                                const char* command,
                                const char* format,
                                ...);

/* ============================================================================
 * Asynchronous Operations
 * ========================================================================= */

vt_event_monitor_handle_t vt_socket_start_monitoring(vt_socket_handle_t handle,
                                                      vt_event_callback_t callback,
                                                      void* user_data);
void vt_socket_stop_monitoring(vt_event_monitor_handle_t monitor);
bool vt_socket_wait_for_event(vt_socket_handle_t handle,
                               const char* event_name,
                               uint32_t timeout_ms,
                               char* data_out,
                               size_t data_size);

/* ============================================================================
 * LCD Event Monitoring (Unity Test Hooks)
 * ========================================================================= */

void vt_socket_register_lcd_callback(vt_event_callback_t callback,
                                     void* user_data);
bool vt_socket_get_lcd(vt_socket_handle_t handle, char lcd_state[VT_LCD_MAX_ROWS][VT_LCD_MAX_COLS]);
bool vt_socket_wait_for_lcd_update(uint32_t timeout_ms, vt_lcd_update_t* lcd_update);

/* ============================================================================
 * High-Level Commands
 * ========================================================================= */

bool vt_cpu_halt(vt_socket_handle_t handle);
bool vt_cpu_run(vt_socket_handle_t handle);
bool vt_cpu_get_status(vt_socket_handle_t handle, vt_cpu_status_t* status);
bool vt_cpu_get_registers(vt_socket_handle_t handle, vt_registers_t* registers);
bool vt_cpu_set_registers(vt_socket_handle_t handle, const vt_registers_t* registers);
bool vt_memory_read(vt_socket_handle_t handle, uint16_t address, uint8_t* data, size_t length);
bool vt_memory_write(vt_socket_handle_t handle, uint16_t address, const uint8_t* data, size_t length);

/* ============================================================================
 * LCD Commands
 * ========================================================================= */

bool vt_lcd_clear(vt_socket_handle_t handle);
bool vt_lcd_write(vt_socket_handle_t handle, uint8_t row, uint8_t col, const char* data);

#ifdef __cplusplus
}
#endif

#endif /* VT_SOCKET_H */