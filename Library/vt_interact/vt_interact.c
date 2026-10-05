/* VirtualT Interact Library - Implementation */

#include "vt_interact.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#endif

/* Error message storage */
static char last_error[256] = "";

/* Function prototypes for helper functions */
static void vt_interact_set_error(const char* msg);
const char* vt_interact_get_error(void);

typedef struct vt_interact_t {
    vt_process_handle_t process;
    vt_socket_handle_t socket;
    bool is_connected;
    uint16_t port;
} vt_interact_internal_t;

/* Helper function to get full ROM path */
static void get_rom_path(vt_interact_handle_t handle, char* buffer, size_t size) {
    vt_interact_internal_t* internal = (vt_interact_internal_t*)handle;
    if (!internal || !buffer || size == 0) return;
    
    const char* vt_path = getenv("VIRTUALT_PATH");
    if (!vt_path) vt_path = ".";
    
    strncpy_s(buffer, size, vt_path, size - 1);
    
    /* Remove trailing backslash */
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\\') {
        buffer[len - 1] = '\0';
    }
    
    /* Append ROM path */
    strncat_s(buffer, size, "\\..\\..\\SUT\\SUPNEC.bin", size - strlen(buffer) - 1);
}

vt_interact_handle_t vt_interact_create(void) {
    vt_interact_internal_t* handle = malloc(sizeof(vt_interact_internal_t));
    if (!handle) return NULL;
    
    handle->process = vt_process_create();
    if (!handle->process) {
        free(handle);
        return NULL;
    }
    
    handle->socket = NULL;
    handle->is_connected = false;
    handle->port = 0;
    
    return (vt_interact_handle_t)handle;
}

void vt_interact_destroy(vt_interact_handle_t handle) {
    vt_interact_internal_t* internal = (vt_interact_internal_t*)handle;
    if (!internal) return;
    
    /* Disconnect socket first */
    if (internal->socket) {
        vt_interact_disconnect(handle);
    }
    
    /* Terminate process if running */
    if (internal->process) {
        vt_process_terminate(internal->process);
        vt_process_destroy(internal->process);
        internal->process = NULL;
    }
    
    free(handle);
}

/* Process Management Functions */
bool vt_interact_launch(vt_interact_handle_t handle, const vt_interact_config_t* config) {
    vt_interact_internal_t* internal = (vt_interact_internal_t*)handle;
    if (!internal || !config) return false;

    vt_process_config_t proc_config = {
        .virtualt_path = config->virtualt_path,
        .port = config->port,
        .headless = config->headless,
        .startup_timeout_ms = config->startup_timeout_ms
    };

    if (!vt_process_launch(internal->process, &proc_config)) {
        vt_interact_set_error(vt_process_get_error());
        return false;
    }

    internal->port = vt_process_get_port(internal->process);
    return true;
}

bool vt_interact_wait_for_ready(vt_interact_handle_t handle, uint32_t timeout_ms) {
    vt_interact_internal_t* internal = (vt_interact_internal_t*)handle;
    if (!internal) return false;

    if (!vt_process_wait_for_ready(internal->process, timeout_ms)) {
        vt_interact_set_error(vt_process_get_error());
        return false;
    }

    return true;
}

/* Socket Connection Functions */
bool vt_interact_connect(vt_interact_handle_t handle) {
    vt_interact_internal_t* internal = (vt_interact_internal_t*)handle;
    if (!internal) return false;

    if (internal->socket) {
        vt_socket_destroy(internal->socket);
    }

    internal->socket = vt_process_create_socket(internal->process, NULL);
    if (!internal->socket) {
        vt_interact_set_error("Failed to create socket for connection");
        return false;
    }

    if (!vt_socket_connect(internal->socket)) {
        vt_socket_destroy(internal->socket);
        internal->socket = NULL;
        vt_interact_set_error("Failed to connect to VirtualT socket");
        return false;
    }

    internal->is_connected = true;
    return true;
}

void vt_interact_disconnect(vt_interact_handle_t handle) {
    vt_interact_internal_t* internal = (vt_interact_internal_t*)handle;
    if (!internal) return;
    
    if (internal->socket) {
        vt_socket_disconnect(internal->socket);
        vt_socket_destroy(internal->socket);
        internal->socket = NULL;
    }
    internal->is_connected = false;
}

bool vt_interact_is_connected(vt_interact_handle_t handle) {
    vt_interact_internal_t* internal = (vt_interact_internal_t*)handle;
    if (!internal) return false;
    return internal->is_connected;
}

bool vt_interact_set_model(vt_interact_handle_t handle, const char* model) {
    vt_interact_internal_t* internal = (vt_interact_internal_t*)handle;
    if (!internal || !internal->socket || !model) {
        vt_interact_set_error("Invalid arguments to vt_interact_set_model");
        return false;
    }

    char command[64];
    snprintf(command, sizeof(command), "model %s", model);

    char response[256];
    if (!vt_socket_send_command(internal->socket, command, response, sizeof(response))) {
        vt_interact_set_error("Failed to set VirtualT model");
        return false;
    }

    return true;
}

bool vt_interact_get_status(vt_interact_handle_t handle, vt_interact_status_t* status) {
    vt_interact_internal_t* internal = (vt_interact_internal_t*)handle;
    if (!internal || !status) return false;
    
    vt_process_status_t proc_status;
    if (!vt_process_get_status(internal->process, &proc_status)) {
        return false;
    }
    
    status->is_running = proc_status.is_running;
    status->is_connected = internal->is_connected;
    status->pc = 0;
    status->sp = 0;
    
    /* Get current PC if connected */
    if (internal->socket) {
        char response[256];
        if (vt_socket_send_command(internal->socket, "pc", response, sizeof(response))) {
            status->pc = (uint16_t)atoi(response);
        }
        if (vt_socket_send_command(internal->socket, "sp", response, sizeof(response))) {
            status->sp = (uint16_t)atoi(response);
        }
    }
    
    return true;
}

bool vt_interact_halt_cpu(vt_interact_handle_t handle) {
    vt_interact_internal_t* internal = (vt_interact_internal_t*)handle;
    if (!internal || !internal->socket) return false;
    
    char response[64];
    return vt_socket_send_command(internal->socket, "halt", response, sizeof(response));
}

bool vt_interact_run_cpu(vt_interact_handle_t handle) {
    vt_interact_internal_t* internal = (vt_interact_internal_t*)handle;
    if (!internal || !internal->socket) return false;
    
    char response[64];
    return vt_socket_send_command(internal->socket, "run", response, sizeof(response));
}

/* LCD Monitoring Functions */
bool vt_interact_lcd_monitor_enable(vt_interact_handle_t handle, bool enable) {
    vt_interact_internal_t* internal = (vt_interact_internal_t*)handle;
    if (!internal || !internal->socket) return false;

    char response[64];
    const char* cmd = enable ? "lcd_mon on" : "lcd_mon off";
    bool result = vt_socket_send_command(internal->socket, cmd, response, sizeof(response));
    return result;
}

bool vt_interact_get_lcd_event(vt_interact_handle_t handle, uint8_t* row, uint8_t* col, char* data, size_t data_size) {
    vt_interact_internal_t* internal = (vt_interact_internal_t*)handle;
    if (!internal || !internal->socket || !row || !col || !data || data_size == 0) return false;

    /* Check for pending LCD write events */
    while (vt_socket_has_pending_events()) {
        char event[256];
        if (vt_socket_pop_event(event, sizeof(event))) {
            /* Format: "event, lcdwrite, (r,c),data" */
            /* Example: "event, lcdwrite, (0,5),H" */
            char* comma1 = strchr(event, ',');
            if (comma1) {
                char* comma2 = strchr(comma1 + 1, ',');
                if (comma2) {
                    /* Find opening parenthesis for (r,c) */
                    char* paren_open = strchr(comma2 + 1, '(');
                    if (paren_open) {
                        char* comma_in_parens = strchr(paren_open, ',');
                        if (comma_in_parens) {
                            char* paren_close = strchr(comma_in_parens, ')');
                            if (paren_close) {
                                /* Extract row */
                                *paren_open = '\0';
                                *paren_close = '\0';
                                *comma_in_parens = '\0';
                                *row = (uint8_t)atoi(paren_open + 1);
                                *col = (uint8_t)atoi(comma_in_parens + 1);

                                /* Copy character data after ) */
                                strncpy_s(data, data_size, paren_close + 1, data_size - 1);
                                data[data_size - 1] = '\0';

                                /* Restore characters */
                                *paren_open = '(';
                                *paren_close = ')';
                                *comma_in_parens = ',';
                                return true;
                            }
                        }
                    }
                }
            }
        }
    }

    return false;
}

/* PC-8201A Specific Operations */
bool vt_interact_load_rom(vt_interact_handle_t handle) {
    vt_interact_internal_t* internal = (vt_interact_internal_t*)handle;
    if (!internal || !internal->socket) {
        vt_interact_set_error("Invalid arguments to vt_interact_load_rom");
        return false;
    }

    char rom_path[512];
    get_rom_path(handle, rom_path, sizeof(rom_path));

    char command[600];
    snprintf(command, sizeof(command), "optrom %s", rom_path);

    char response[512];
    if (!vt_socket_send_command(internal->socket, command, response, sizeof(response))) {
        vt_interact_set_error("Failed to load ROM");
        return false;
    }

    return true;
}

bool vt_interact_launch_basic(vt_interact_handle_t handle) {

    // TODO: reimplement. This is nonsense.
    //       You cannot tell if the interactive interpreter is running by checking the PC.
    //   The PC will be 0x0000 on reset, but it will also be 0x0000 if the CPU is halted at the reset vector.
    //   You need to check for a specific memory location or a specific response from the interpreter to determine if it is running.
    //   Also there are two modes: interactive, or running a BASIC program. You need to check for both.
    //   Probably since this function "launches BASIC" it would only ever
    //   be invoked at the main menu. So it should navigate to the BASIC menu option
    // and press ENTER to launch it. Then it should check for the BASIC prompt to verify that BASIC is running.


    #if 0
    /* The BASIC interpreter is loaded with the ROM */
    /* This function could be used to verify BASIC is running */
    vt_interact_internal_t* internal = (vt_interact_internal_t*)handle;
    if (!internal || !internal->socket) return false;
    
    /* Verify we're in BASIC by checking PC (BASIC entry point) */
    char response[256];
    if (vt_socket_send_command(internal->socket, "pc", response, sizeof(response))) {
        uint16_t pc = (uint16_t)atoi(response);
        /* BASIC starts at 0x0000 on reset */
        if (pc == 0x0000 || pc == 0x100) {
            return true;
        }
    }
    #endif
    
    return false;
}

/* Escape quotes in a string for the VirtualT 'key' command */
static void escape_quotes(const char* src, char* dst, size_t dst_size) {
    size_t src_idx = 0;
    size_t dst_idx = 0;

    while (src[src_idx] != '\0' && dst_idx + 2 < dst_size) {
        if (src[src_idx] == '"' || src[src_idx] == '\\') {
            /* Escape quote and backslash with backslash */
            dst[dst_idx++] = '\\';
        }
        dst[dst_idx++] = src[src_idx];
        src_idx++;
    }
    dst[dst_idx] = '\0';
}

bool vt_interact_type_line(vt_interact_handle_t handle, const char* line) {
    /* Type a line of BASIC using the VirtualT 'key' command */
    vt_interact_internal_t* internal = (vt_interact_internal_t*)handle;
    if (!internal || !internal->socket || !line) return false;

    /* Escape quotes in the line (double them for VirtualT) */
    char escaped_line[512];
    escape_quotes(line, escaped_line, sizeof(escaped_line));

    char command[512];
    snprintf(command, sizeof(command), "key \"%s\"", escaped_line);

    char response[1024];
    memset(response, 0, sizeof(response));
    bool result = vt_socket_send_command(internal->socket, command, response, sizeof(response));

    /* Small delay to allow keystrokes to be processed */
    if (result) {
#ifdef _WIN32
        Sleep(100);
#else
        usleep(100000);
#endif
    }

    return result;
}

bool vt_interact_execute_program(vt_interact_handle_t handle) {
    /* Send ENTER to execute current line or run program */
    vt_interact_internal_t* internal = (vt_interact_internal_t*)handle;
    if (!internal || !internal->socket) return false;
    
    /* Send ENTER key to execute the current line or run program */
    char response[256];
    bool result = vt_socket_send_command(internal->socket, "key enter", response, sizeof(response));
    
    /* Small delay to allow command to execute */
    if (result) {
#ifdef _WIN32
        Sleep(100);
#else
        usleep(100000);
#endif
    }
    
    return result;
}

/* Helper Functions */
uint16_t vt_interact_get_port(vt_interact_handle_t handle) {
    vt_interact_internal_t* internal = (vt_interact_internal_t*)handle;
    if (!internal) return 0;
    return internal->port;
}

bool vt_interact_is_running(vt_interact_handle_t handle) {
    vt_interact_internal_t* internal = (vt_interact_internal_t*)handle;
    if (!internal) return false;
    return vt_process_is_running(internal->process);
}

bool vt_interact_terminate(vt_interact_handle_t handle) {
    vt_interact_internal_t* internal = (vt_interact_internal_t*)handle;
    if (!internal) return false;
    
    if (internal->socket) {
        vt_interact_disconnect(handle);
    }
    
    if (internal->process) {
        bool result = vt_process_terminate(internal->process);
        vt_process_destroy(internal->process);
        internal->process = NULL;
        return result;
    }
    
    return false;
}

/* Helper Functions */
const char* vt_interact_get_error(void) {
    return last_error;
}

static void vt_interact_set_error(const char* msg) {
    strncpy_s(last_error, sizeof(last_error), msg, sizeof(last_error) - 1);
}

/* Logging function that always flushes */
static void vt_interact_log(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    printf("\n");
    fflush(stdout);
    va_end(args);
}

/* ============================================================================
 * Helper Functions for PC-8201A Memory Operations
 * ========================================================================= */

/* Read cursor position from PC-8201A memory */
/* Returns: true on success, stores x (0-39) and y (0-7) */
static bool get_cursor_position(vt_interact_handle_t handle, uint8_t* x, uint8_t* y) {
    vt_interact_internal_t* internal = (vt_interact_internal_t*)handle;
    if (!internal || !internal->socket || !x || !y) {
        vt_interact_set_error("Invalid arguments to get_cursor_position");
        return false;
    }

    char response[256];

    /* Read X cursor position (0xF3E6) */
    if (!vt_socket_send_command(internal->socket, "radix 10", response, sizeof(response))) {
        vt_interact_set_error("Failed to set radix");
        return false;
    }

    char cmd[64];
    snprintf(cmd, sizeof(cmd), "rm %d 1", VT_PC8201_CURSOR_X);
    vt_interact_log("DEBUG: Sending cursor X command: %s", cmd);
    if (!vt_socket_send_command(internal->socket, cmd, response, sizeof(response))) {
        vt_interact_set_error("Failed to read cursor X position");
        return false;
    }

    /* Parse the response - should be a single decimal number */
    int x_val = atoi(response);
    vt_interact_log("DEBUG: Cursor X value: %d", x_val);
    /* PC-8201A cursor X is 1-40 (1-based), convert to 0-39 for internal use */
    if (x_val < 1 || x_val > 40) {
        vt_interact_set_error("Invalid cursor X position");
        return false;
    }
    *x = (uint8_t)(x_val - 1);  /* Convert to 0-based */

    /* Read Y cursor position (0xF3E5) */
    snprintf(cmd, sizeof(cmd), "rm %d 1", VT_PC8201_CURSOR_Y);
    vt_interact_log("DEBUG: Sending cursor Y command: %s", cmd);
    if (!vt_socket_send_command(internal->socket, cmd, response, sizeof(response))) {
        vt_interact_set_error("Failed to read cursor Y position");
        return false;
    }

    int y_val = atoi(response);
    vt_interact_log("DEBUG: Cursor Y value: %d", y_val);
    /* PC-8201A cursor Y is 1-8 (1-based), convert to 0-7 for internal use */
    if (y_val < 1 || y_val > 8) {
        vt_interact_set_error("Invalid cursor Y position");
        return false;
    }
    *y = (uint8_t)(y_val - 1);  /* Convert to 0-based */

    return true;
}

/* Parse decimal byte values from rm command response */
/* Response format: "byte1 byte2 byte3 ... byteN" */
static int parse_rm_response(const char* response, uint8_t* buffer, size_t buffer_size) {
    vt_interact_log("DEBUG: parse_rm_response, response length=%zu", strlen(response));
    size_t pos = 0;
    char* temp = _strdup(response);
    if (!temp) return 0;

    char* saveptr = NULL;
    char* token = strtok_s(temp, " \n\r\t", &saveptr);
    vt_interact_log("DEBUG: Starting to parse tokens");
    while (token != NULL && pos < buffer_size) {
        int byte_val = atoi(token);
        if (byte_val >= 0 && byte_val <= 255) {
            buffer[pos++] = (uint8_t)byte_val;
        }
        token = strtok_s(saveptr, " \n\r\t", &saveptr);
    }
    vt_interact_log("DEBUG: Parsed %zu bytes", pos);

    free(temp);
    return (int)pos;
}

/* Read LCD memory from specified address */
static bool read_lcd_memory(vt_interact_handle_t handle, uint16_t address, uint8_t* buffer, size_t length) {
    vt_interact_log("DEBUG: read_lcd_memory(address=%d, length=%zu)", address, length);
    vt_interact_internal_t* internal = (vt_interact_internal_t*)handle;
    if (!internal || !internal->socket || !buffer || length == 0) {
        vt_interact_set_error("Invalid arguments to read_lcd_memory");
        return false;
    }

    char response[4096];

    /* Set radix to 10 (decimal) */
    vt_interact_log("DEBUG: Setting radix to 10");
    if (!vt_socket_send_command(internal->socket, "radix 10", response, sizeof(response))) {
        vt_interact_set_error("Failed to set radix");
        return false;
    }

    /* Read memory using rm command */
    char cmd[64];
    snprintf(cmd, sizeof(cmd), "rm %u %zu", address, length);
    vt_interact_log("DEBUG: Sending rm command: %s", cmd);

    if (!vt_socket_send_command(internal->socket, cmd, response, sizeof(response))) {
        vt_interact_log("DEBUG: rm command failed");
        vt_interact_set_error("Failed to read LCD memory");
        return false;
    }
    vt_interact_log("DEBUG: rm command succeeded, response length=%zu", strlen(response));

    /* Parse response: "byte1 byte2 byte3 ..." -> array of bytes */
    int count = parse_rm_response(response, buffer, length);
    vt_interact_log("DEBUG: Parsed %d bytes", count);
    return count == (int)length;
}

/* Convert LCD memory bytes to string, filtering non-printable chars */
static void lcd_bytes_to_string(const uint8_t* bytes, size_t length, char* output, size_t output_size) {
    size_t pos = 0;
    for (size_t i = 0; i < length && pos < output_size - 1; i++) {
        uint8_t byte = bytes[i];
        /* Skip null bytes and backspace */
        if (byte == 0x00 || byte == 0x08) {
            continue;
        }
        /* Only include printable ASCII */
        if (byte >= 32 && byte < 127) {
            output[pos++] = (char)byte;
        }
    }
    output[pos] = '\0';
}

/* Menu Navigation Functions */
bool vt_interact_menu_navigate(vt_interact_handle_t handle, uint8_t row, uint8_t current_row) {
    vt_interact_internal_t* internal = (vt_interact_internal_t*)handle;
    if (!internal || !internal->socket) return false;
    
    /* Determine direction and number of presses */
    char response[256];
    bool result = true;
    
    if (row > current_row) {
        /* Down arrow presses */
        for (int i = 0; i < (row - current_row) && result; i++) {
            result = vt_socket_send_command(internal->socket, "key down", response, sizeof(response));
            if (result) {
#ifdef _WIN32
                Sleep(50);
#else
                usleep(50000);
#endif
            }
        }
    } else if (row < current_row) {
        /* Up arrow presses */
        for (int i = 0; i < (current_row - row) && result; i++) {
            result = vt_socket_send_command(internal->socket, "key up", response, sizeof(response));
            if (result) {
#ifdef _WIN32
                Sleep(50);
#else
                usleep(50000);
#endif
            }
        }
    }
    
    return result;
}

bool vt_interact_launch_basic_from_menu(vt_interact_handle_t handle) {
    vt_interact_internal_t* internal = (vt_interact_internal_t*)handle;
    if (!internal || !internal->socket) return false;
    
    /* In the PC-8201A main menu, navigate to BASIC (typically 1st or 2nd entry)
     * Then press ENTER to launch it */
    char response[256];
    bool result = true;
    
    /* Navigate to BASIC (row 0) */
    if (result) {
        result = vt_socket_send_command(internal->socket, "key up", response, sizeof(response));
        if (result) Sleep(100);
    }
    
    /* Press ENTER to launch BASIC */
    if (result) {
        result = vt_socket_send_command(internal->socket, "key enter", response, sizeof(response));
    }
    
    return result;
}

bool vt_interact_type_program(vt_interact_handle_t handle, const char* lines[], int count) {
    vt_interact_internal_t* internal = (vt_interact_internal_t*)handle;
    if (!internal || !internal->socket || !lines || count <= 0) return false;
    
    char response[256];
    bool result = true;
    
    for (int i = 0; i < count && result; i++) {
        result = vt_interact_type_line(handle, lines[i]);
        if (result) {
            /* Add ENTER to execute the line */
            result = vt_socket_send_command(internal->socket, "key enter", response, sizeof(response));
            if (result) {
#ifdef _WIN32
                Sleep(100);
#else
                usleep(100000);
#endif
            }
        }
    }
    
    return result;
}

bool vt_interact_exit_to_menu(vt_interact_handle_t handle) {
    vt_interact_internal_t* internal = (vt_interact_internal_t*)handle;
    if (!internal || !internal->socket) return false;
    
    /* Send F8 to exit back to the main menu */
    char response[256];
    return vt_socket_send_command(internal->socket, "key f8", response, sizeof(response));
}

bool vt_interact_press_key(vt_interact_handle_t handle, const char* key_name) {
    vt_interact_internal_t* internal = (vt_interact_internal_t*)handle;
    if (!internal || !internal->socket || !key_name) return false;
    
    char command[256];
    snprintf(command, sizeof(command), "key %s", key_name);
    
    char response[256];
    bool result = vt_socket_send_command(internal->socket, command, response, sizeof(response));
    
    /* Small delay to allow keystroke to be processed */
    if (result) {
#ifdef _WIN32
        Sleep(50);
#else
        usleep(50000);
#endif
    }
    
    return result;
}

bool vt_interact_type_text(vt_interact_handle_t handle, const char* text, bool newline) {
    vt_interact_internal_t* internal = (vt_interact_internal_t*)handle;
    if (!internal || !internal->socket || !text) return false;

    char command[512];

    /* Escape special characters in text for the key command */
    /* For now, just pass the text directly */
    snprintf(command, sizeof(command), "key \"%s\"", text);
    
    char response[256];
    bool result = vt_socket_send_command(internal->socket, command, response, sizeof(response));
    
    /* Add newline if requested */
    if (result && newline) {
        result = vt_socket_send_command(internal->socket, "key enter", response, sizeof(response));
        if (result) {
#ifdef _WIN32
            Sleep(100);
#else
            usleep(100000);
#endif
        }
    } else if (result) {
        /* Small delay to allow typing to complete */
#ifdef _WIN32
        Sleep(100);
#else
        usleep(100000);
#endif
    }

    return result;
}

/* Internal function to get socket from handle */
static vt_socket_handle_t get_socket(vt_interact_handle_t handle) {
    vt_interact_internal_t* internal = (vt_interact_internal_t*)handle;
    if (!internal) return NULL;
    return internal->socket;
}

bool vt_interact_send_command(vt_interact_handle_t handle, const char* command, char* response, size_t response_size) {
    vt_socket_handle_t socket = get_socket(handle);
    if (!socket || !command || !response || response_size == 0) {
        vt_interact_set_error("Invalid arguments to vt_interact_send_command");
        return false;
    }

    return vt_socket_send_command(socket, command, response, response_size);
}

bool vt_interact_verify_mem_contains(vt_interact_handle_t handle, uint16_t address, size_t length, const char* expected_content) {
    vt_socket_handle_t socket = get_socket(handle);
    if (!socket || !expected_content || length == 0) {
        vt_interact_set_error("Invalid arguments to vt_interact_verify_mem_contains");
        return false;
    }

    char response[4096];

    /* Set radix to 10 (decimal) - default and works consistently */
    vt_socket_send_command(socket, "radix 10", response, sizeof(response));

    char command[256];
    /* Read memory using the 'rm' command with decimal address */
    snprintf(command, sizeof(command), "rm %u %zu", address, length);

    if (!vt_socket_send_command(socket, command, response, sizeof(response))) {
        printf("Failed to read memory at address %u\n", address);
        printf("Command sent: %s\n", command);
        printf("Response received: %s\n", response);
        return false;
    }

    printf("Memory read from address %u (0x%04X) (length %zu):\n%s\n", address, address, length, response);

    /* The rm command returns decimal byte values separated by spaces
     * We need to search for the expected content in ASCII form within the response
     * The memory we're reading is the PC-8201 LCD VRAM which contains ASCII characters
     * Convert the decimal byte values to ASCII for searching */

    /* Parse the response to extract ASCII characters from the decimal values */
    char ascii_buffer[4096];
    size_t ascii_pos = 0;

    /* Parse decimal byte values and convert to ASCII */
    /* The response format is: "byte1 byte2 byte3 ... byteN" */
    char* temp = _strdup(response);
    if (temp) {
        char* token = strtok_s(temp, " \n", &temp);
        while (token != NULL) {
            int byte_val = atoi(token);
            if (byte_val >= 32 && byte_val <= 126) {  /* Printable ASCII */
                if (ascii_pos < sizeof(ascii_buffer) - 1) {
                    ascii_buffer[ascii_pos++] = (char)byte_val;
                }
            }
            token = strtok_s(temp, " \n", &temp);
        }
        free(temp);
    }
    ascii_buffer[ascii_pos] = '\0';

    printf("Parsed ASCII from memory (first 150 chars): %s\n", ascii_buffer);
    printf("Parsed ASCII length: %zu bytes\n", ascii_pos);

    /* Check if expected content is in the parsed ASCII */
    bool found = strstr(ascii_buffer, expected_content) != NULL;

    if (found) {
        printf("Found expected content in memory at address %u: '%s'\n", address, expected_content);
    } else {
        printf("Expected content not found in parsed ASCII from memory at address %u: '%s'\n", address, expected_content);
        /* Try searching in the raw response for a hint */
        printf("Raw response contains 'My First Outline' check: %d\n", strstr(response, "77 121 32 70 105 114 115 116 32 79 117 116 108 105 110 101") != NULL);
    }

    return found;
}

/* ============================================================================
 * Main Keystroke Sending Function
 * ========================================================================= */

bool vt_interact_send_keystrokes(vt_interact_handle_t handle,
                                const char* keystrokes[],
                                int keystroke_count,
                                uint32_t timeout_ms,
                                const char* expected_output,
                                bool reset,
                                int* error_code) {
    vt_interact_internal_t* internal = (vt_interact_internal_t*)handle;
    vt_interact_log("DEBUG: vt_interact_send_keystrokes entered");

    /* Validate handle and socket */
    if (!internal || !internal->socket) {
        vt_interact_set_error("Not connected to VirtualT");
        if (error_code) *error_code = VT_INTERACT_CONNECTION_ERR;
        return false;
    }

    if (!keystrokes || keystroke_count <= 0) {
        vt_interact_set_error("Invalid keystrokes array");
        if (error_code) *error_code = VT_INTERACT_ERROR;
        return false;
    }

    if (!expected_output) {
        vt_interact_set_error("Expected output cannot be NULL");
        if (error_code) *error_code = VT_INTERACT_ERROR;
        return false;
    }

    uint8_t cursor_before_x = 0, cursor_before_y = 0;
    uint8_t cursor_after_x = 0, cursor_after_y = 0;

    /* If not reset, read cursor position before sending keystrokes */
    if (!reset) {
        vt_interact_log("DEBUG: Reading cursor position before");
        if (!get_cursor_position(handle, &cursor_before_x, &cursor_before_y)) {
            if (error_code) *error_code = VT_INTERACT_ERROR;
            return false;
        }
        vt_interact_log("DEBUG: Cursor before: x=%d, y=%d", cursor_before_x, cursor_before_y);
    }

    /* Send keystrokes */
    char response[256];
    for (int i = 0; i < keystroke_count; i++) {
        const char* key = keystrokes[i];
        if (!key) {
            vt_interact_set_error("NULL keystroke in array");
            if (error_code) *error_code = VT_INTERACT_ERROR;
            return false;
        }

        /* Build and send key command */
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "key \"%s\"", key);
        vt_interact_log("DEBUG: Sending keystroke: %s", cmd);

        if (!vt_socket_send_command(internal->socket, cmd, response, sizeof(response))) {
            vt_interact_set_error("Failed to send keystroke");
            if (error_code) *error_code = VT_INTERACT_ERROR;
            return false;
        }
        vt_interact_log("DEBUG: Keystroke sent successfully");

        /* Small delay to allow keystroke to be processed */
#ifdef _WIN32
        Sleep(50);
#else
        usleep(50000);
#endif
    }

    /* Wait for timeout_ms for processing */
    vt_interact_log("DEBUG: Waiting for %d ms for processing", timeout_ms);
    uint32_t elapsed = 0;
    while (elapsed < timeout_ms) {
#ifdef _WIN32
        Sleep(10);
#else
        usleep(10000);
#endif
        elapsed += 10;
    }
    vt_interact_log("DEBUG: Processing wait complete");

    /* Read cursor position after keystrokes */
    vt_interact_log("DEBUG: Reading cursor position after");
    if (!get_cursor_position(handle, &cursor_after_x, &cursor_after_y)) {
        if (error_code) *error_code = VT_INTERACT_ERROR;
        return false;
    }
    vt_interact_log("DEBUG: Cursor after: x=%d, y=%d", cursor_after_x, cursor_after_y);

    /* Read LCD memory based on reset flag */
    uint8_t lcd_buffer[320];
    size_t read_length;
    uint16_t read_address;

    if (reset) {
        /* Read from start of VRAM to new cursor position */
        read_address = VT_PC8201_LCD_VRAM;
        /* Calculate bytes to read: from start to cursor */
        /* Cursor is at (cursor_after_x, cursor_after_y), 40 chars per row */
        uint16_t cursor_pos = (uint16_t)(cursor_after_y * 40 + cursor_after_x);
        read_length = (cursor_pos < sizeof(lcd_buffer)) ? cursor_pos : (uint16_t)sizeof(lcd_buffer);
        vt_interact_log("DEBUG: Read address=%d, length=%zu", read_address, read_length);

        /* For reset=true, we expect the typed text to be at the end of the read */
        /* Adjust to read only the new characters if cursor moved more than expected */
        size_t expected_max_length = (size_t)(cursor_before_x + cursor_before_y * 40);
        if (read_length > expected_max_length + keystroke_count) {
            /* Cursor moved more than expected (line wraps, etc.)
               Read only the new characters from the current cursor position */
            read_address = VT_PC8201_LCD_VRAM + (cursor_pos - keystroke_count);
            read_length = keystroke_count;
        }
    } else {
        /* Read from old cursor position to new cursor position */
        uint16_t start_pos = (uint16_t)(cursor_before_y * 40 + cursor_before_x);
        uint16_t end_pos = (uint16_t)(cursor_after_y * 40 + cursor_after_x);

        if (end_pos >= start_pos) {
            read_address = VT_PC8201_LCD_VRAM + start_pos;
            read_length = end_pos - start_pos;
        } else {
            /* Wrapping occurred (line wrap or screen wrap) */
            /* Read from start_pos to end, then from beginning to end_pos */
            read_address = VT_PC8201_LCD_VRAM + start_pos;
            read_length = (uint16_t)(sizeof(lcd_buffer) - start_pos);  /* Read to end */
        }

        /* Cap read length */
        if (read_length > sizeof(lcd_buffer)) {
            read_length = (uint16_t)sizeof(lcd_buffer);
        }
        vt_interact_log("DEBUG: Read address=%d, length=%zu", read_address, read_length);
    }

    if (read_length == 0) {
        vt_interact_set_error("No bytes to read from LCD memory");
        if (error_code) *error_code = VT_INTERACT_ERROR;
        return false;
    }

    vt_interact_log("DEBUG: Reading LCD memory");
    if (!read_lcd_memory(handle, read_address, lcd_buffer, read_length)) {
        if (error_code) *error_code = VT_INTERACT_ERROR;
        return false;
    }
    vt_interact_log("DEBUG: LCD memory read complete");

    /* Convert to string, filtering null and backspace */
    char actual_output[320];
    lcd_bytes_to_string(lcd_buffer, read_length, actual_output, sizeof(actual_output));
    vt_interact_log("DEBUG: Actual output: '%s'", actual_output);

    /* Compare with expected output */
    if (strcmp(actual_output, expected_output) != 0) {
        vt_interact_set_error("Display output mismatch");
        if (error_code) *error_code = VT_INTERACT_MATCH_ERR;
        return false;
    }

    vt_interact_log("DEBUG: Test passed!");
    return true;
}