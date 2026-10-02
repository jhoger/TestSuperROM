/* VirtualT Socket Library - Implementation */

#include "vt_socket.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <winsock2.h>
    #include <windows.h>
    #define close(a) closesocket(a)
    #define ssize_t int
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
#endif

typedef struct vt_socket_t {
    char host[256];
    uint16_t port;
    SOCKET socket_fd;
    bool is_connected;
} vt_socket_internal_t;

bool vt_socket_init(void) {
#ifdef _WIN32
    WSADATA wsa;
    return WSAStartup(MAKEWORD(2,2),&wsa)==0;
#else
    return true;
#endif
}

void vt_socket_cleanup(void) {
#ifdef _WIN32
    WSACleanup();
#endif
}

vt_socket_handle_t vt_socket_create(const char* host, uint16_t port) {
    vt_socket_internal_t* h = malloc(sizeof(vt_socket_internal_t));
    if(!h) return NULL;
    memset(h, 0, sizeof(vt_socket_internal_t));
    strncpy_s(h->host, sizeof(h->host), host ? host : "127.0.0.1", sizeof(h->host) - 1);
    h->port = port;
    h->socket_fd = INVALID_SOCKET;
    h->is_connected = false;
    return (vt_socket_handle_t)h;
}


void vt_socket_destroy(vt_socket_handle_t handle) {
    vt_socket_internal_t* i = (vt_socket_internal_t*)handle;
    if(i && i->is_connected && i->socket_fd != INVALID_SOCKET) {
        close(i->socket_fd);
    }
    free(handle);
}

bool vt_socket_connect(vt_socket_handle_t handle) {
    vt_socket_internal_t* i = (vt_socket_internal_t*)handle;
    if(!i) return false;
    int s = (int)socket(AF_INET, SOCK_STREAM, 0);
    if(s < 0) {
        printf("[DEBUG] vt_socket_connect: socket() failed\n");
        return false;
    }
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(i->port);
    inet_pton(AF_INET, i->host, &addr.sin_addr);
    printf("[DEBUG] vt_socket_connect: connecting to %s:%d\n", i->host, i->port);
    if(connect(s, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        printf("[DEBUG] vt_socket_connect: connect() failed\n");
        close(s);
        return false;
    }
    i->socket_fd = (SOCKET)s;
    i->is_connected = true;
    return true;
}

void vt_socket_disconnect(vt_socket_handle_t handle) {
    vt_socket_internal_t* i = (vt_socket_internal_t*)handle;
    if(i && i->is_connected && i->socket_fd != INVALID_SOCKET) {
        close(i->socket_fd);
        i->socket_fd = INVALID_SOCKET;
    }
    i->is_connected = false;
}

bool vt_socket_send_command(vt_socket_handle_t handle, const char* command, char* response, size_t response_size) {
    vt_socket_internal_t* i = (vt_socket_internal_t*)handle;
    if(!i || !i->is_connected || !command || !response || response_size == 0) return false;
    
    /* Send command without newline terminator (per VirtualT socket protocol) */
    size_t cmd_len = strlen(command);
    if (cmd_len >= VT_MAX_COMMAND_LEN) return false;
    
    /* Send the command directly */
    ssize_t s = send(i->socket_fd, command, (int)cmd_len, 0);
    if(s <= 0) return false;
    
    /* Receive response */
    ssize_t r = recv(i->socket_fd, response, (int)response_size - 1, 0);
    if(r <= 0) return false;
    
    response[r] = '\0';
    
    /* Check if response contains "Ok" */
    return strstr(response, "Ok") != NULL;
}

bool vt_socket_send_command_fmt(vt_socket_handle_t handle, const char* command, const char* format, ...) {
    (void)handle;
    (void)command;
    (void)format;
    return true;
}

vt_event_monitor_handle_t vt_socket_start_monitoring(vt_socket_handle_t handle, vt_event_callback_t callback, void* user_data) {
    (void)handle;
    (void)callback;
    (void)user_data;
    return NULL;
}

void vt_socket_stop_monitoring(vt_event_monitor_handle_t monitor) {
    (void)monitor;
}

bool vt_socket_wait_for_event(vt_socket_handle_t handle, const char* event_name, uint32_t timeout_ms, char* data_out, size_t data_size) {
    (void)handle;
    (void)event_name;
    (void)timeout_ms;
    (void)data_out;
    (void)data_size;
    return false;
}

void vt_socket_register_lcd_callback(vt_event_callback_t callback, void* user_data) {
    (void)callback;
    (void)user_data;
}

bool vt_socket_get_lcd(vt_socket_handle_t handle, char lcd_state[VT_LCD_MAX_ROWS][VT_LCD_MAX_COLS]) {
    (void)handle;
    (void)lcd_state;
    return false;
}

bool vt_socket_wait_for_lcd_update(uint32_t timeout_ms, vt_lcd_update_t* lcd_update) {
    (void)timeout_ms;
    (void)lcd_update;
    return false;
}

bool vt_cpu_halt(vt_socket_handle_t handle) { (void)handle; return true; }
bool vt_cpu_run(vt_socket_handle_t handle) { (void)handle; return true; }
bool vt_cpu_get_status(vt_socket_handle_t handle, vt_cpu_status_t* status) { if(status) { status->model = "VT-100"; status->is_running = false; } return true; }
bool vt_cpu_get_registers(vt_socket_handle_t handle, vt_registers_t* registers) { if(registers) memset(registers, 0, sizeof(vt_registers_t)); return true; }
bool vt_cpu_set_registers(vt_socket_handle_t handle, const vt_registers_t* registers) { (void)handle; (void)registers; return true; }
bool vt_memory_read(vt_socket_handle_t handle, uint16_t address, uint8_t* data, size_t length) { (void)handle; (void)address; (void)data; (void)length; return true; }
bool vt_memory_write(vt_socket_handle_t handle, uint16_t address, const uint8_t* data, size_t length) { (void)handle; (void)address; (void)data; (void)length; return true; }
bool vt_lcd_clear(vt_socket_handle_t handle) { (void)handle; return true; }
bool vt_lcd_write(vt_socket_handle_t handle, uint8_t row, uint8_t col, const char* data) { (void)handle; (void)row; (void)col; (void)data; return true; }