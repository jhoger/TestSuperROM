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
        return false;
    }
    
    internal->port = vt_process_get_port(internal->process);
    return true;
}

bool vt_interact_wait_for_ready(vt_interact_handle_t handle, uint32_t timeout_ms) {
    vt_interact_internal_t* internal = (vt_interact_internal_t*)handle;
    if (!internal) return false;
    
    return vt_process_wait_for_ready(internal->process, timeout_ms);
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
        return false;
    }
    
    if (!vt_socket_connect(internal->socket)) {
        vt_socket_destroy(internal->socket);
        internal->socket = NULL;
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
    if (!internal || !internal->socket || !model) return false;
    
    char response[256];
    return vt_socket_send_command(internal->socket, model, response, sizeof(response));
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

/* LCD Operations */
bool vt_interact_lcd_clear(vt_interact_handle_t handle) {
    vt_interact_internal_t* internal = (vt_interact_internal_t*)handle;
    if (!internal || !internal->socket) return false;
    
    char response[64];
    return vt_socket_send_command(internal->socket, "lcd_clear", response, sizeof(response));
}

bool vt_interact_lcd_write(vt_interact_handle_t handle, uint8_t row, uint8_t col, const char* data) {
    vt_interact_internal_t* internal = (vt_interact_internal_t*)handle;
    if (!internal || !internal->socket || !data) return false;
    
    char command[128];
    snprintf(command, sizeof(command), "lcd %d %d %s", row, col, data);
    
    char response[256];
    return vt_socket_send_command(internal->socket, command, response, sizeof(response));
}

bool vt_interact_lcd_get(vt_interact_handle_t handle, char lcd_state[VT_PC8201_LCD_ROWS][VT_PC8201_LCD_COLS]) {
    (void)handle;
    (void)lcd_state;
    /* LCD read not directly supported by socket interface */
    /* This would require reading LCD memory directly */
    return false;
}

/* PC-8201A Specific Operations */
bool vt_interact_load_rom(vt_interact_handle_t handle) {
    vt_interact_internal_t* internal = (vt_interact_internal_t*)handle;
    if (!internal || !internal->socket) return false;
    
    char rom_path[512];
    get_rom_path(handle, rom_path, sizeof(rom_path));
    
    char command[600];
    snprintf(command, sizeof(command), "optrom %s", rom_path);
    
    char response[512];
    return vt_socket_send_command(internal->socket, command, response, sizeof(response));
}

bool vt_interact_launch_basic(vt_interact_handle_t handle) {
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
    
    return false;
}

bool vt_interact_type_line(vt_interact_handle_t handle, const char* line) {
    /* Type a line of BASIC by writing to keyboard buffer */
    vt_interact_internal_t* internal = (vt_interact_internal_t*)handle;
    if (!internal || !internal->socket || !line) return false;
    
    /* This is a simplified implementation */
    /* In a real implementation, we would need to simulate keyboard input */
    /* by writing to the PC-8201A keyboard buffer memory */
    
    /* For now, we'll just return success as a placeholder */
    /* The actual keyboard input would require: */
    /* 1. Halt CPU */
    /* 2. Write keystrokes to memory locations */
    /* 3. Signal keyboard interrupt */
    /* 4. Resume CPU */
    
    return true;
}

bool vt_interact_execute_program(vt_interact_handle_t handle) {
    /* Send ENTER to execute current line or run program */
    vt_interact_internal_t* internal = (vt_interact_internal_t*)handle;
    if (!internal || !internal->socket) return false;
    
    /* In the NEC PC-8201A, this would involve simulating the ENTER key */
    /* through keyboard buffer manipulation */
    
    return true;
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

const char* vt_interact_get_error(void) {
    return last_error;
}

void vt_interact_set_error(const char* msg) {
    strncpy_s(last_error, sizeof(last_error), msg, sizeof(last_error) - 1);
}