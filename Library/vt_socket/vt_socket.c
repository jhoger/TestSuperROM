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

/* Event queue for async events */
#define VT_MAX_EVENTS 16
#define VT_EVENT_LEN 256
typedef struct {
    char events[VT_MAX_EVENTS][VT_EVENT_LEN];
    int head;
    int tail;
    int count;
} vt_event_queue_t;

static vt_event_queue_t g_event_queue;
static vt_event_callback_t g_event_callback = NULL;
static void* g_event_callback_data = NULL;

#ifdef _WIN32
static CRITICAL_SECTION g_event_cs;
static bool g_cs_initialized = false;
#endif

/* Initialize event queue */
static void event_queue_init(void) {
    memset(&g_event_queue, 0, sizeof(g_event_queue));
#ifdef _WIN32
    if (!g_cs_initialized) {
        InitializeCriticalSection(&g_event_cs);
        g_cs_initialized = true;
    }
#endif
}

/* Add event to queue */
static void event_queue_push(const char* event) {
#ifdef _WIN32
    EnterCriticalSection(&g_event_cs);
#endif

    if (g_event_queue.count < VT_MAX_EVENTS) {
        strcpy_s(g_event_queue.events[g_event_queue.tail], VT_EVENT_LEN, event);
        g_event_queue.tail = (g_event_queue.tail + 1) % VT_MAX_EVENTS;
        g_event_queue.count++;
    }

#ifdef _WIN32
    LeaveCriticalSection(&g_event_cs);
#endif

    /* Call callback if registered */
    if (g_event_callback) {
        /* Parse event: format is "event, event_name, event_data\n" */
        char event_name[64] = "";
        char event_data[256] = "";

        const char* comma1 = strchr(event, ',');
        if (comma1) {
            const char* start = event + 1;  /* Skip leading space */
            const char* comma2 = strchr(comma1 + 1, ',');
            if (comma2) {
                size_t name_len = comma1 - start;
                if (name_len < sizeof(event_name)) {
                    strncpy_s(event_name, sizeof(event_name), start, name_len);
                    event_name[name_len] = '\0';
                }
                strncpy_s(event_data, sizeof(event_data), comma2 + 1, strlen(comma2 + 1));
                /* Remove trailing newline */
                size_t data_len = strlen(event_data);
                if (data_len > 0 && event_data[data_len - 1] == '\n') {
                    event_data[data_len - 1] = '\0';
                }
            }
        }

        g_event_callback(event_name, event_data, g_event_callback_data);
    }
}

/* Pop event from queue */
static bool event_queue_pop(char* event, size_t size) {
#ifdef _WIN32
    EnterCriticalSection(&g_event_cs);
#endif

    if (g_event_queue.count > 0) {
        strncpy_s(event, size, g_event_queue.events[g_event_queue.head], size - 1);
        g_event_queue.head = (g_event_queue.head + 1) % VT_MAX_EVENTS;
        g_event_queue.count--;
#ifdef _WIN32
        LeaveCriticalSection(&g_event_cs);
#endif
        return true;
    }

#ifdef _WIN32
    LeaveCriticalSection(&g_event_cs);
#endif
    return false;
}

/* Check if queue has events */
static bool event_queue_has_events(void) {
#ifdef _WIN32
    EnterCriticalSection(&g_event_cs);
#endif
    bool has = g_event_queue.count > 0;
#ifdef _WIN32
    LeaveCriticalSection(&g_event_cs);
#endif
    return has;
}

/* Helper to extract complete lines from a buffer */
static int extract_lines(const char* buf, char lines[][1024], int max_lines) {
    int count = 0;
    const char* ptr = buf;

    while (count < max_lines && *ptr != '\0') {
        char* line = lines[count];
        char* line_end = line;
        bool has_newline = false;

        /* Copy characters until newline or buffer full */
        while (*ptr != '\0' && *ptr != '\n' && line_end < line + 1023) {
            *line_end++ = *ptr++;
        }
        if (*ptr == '\n') {
            has_newline = true;
            ptr++;
        }
        *line_end = '\0';

        if (line_end > line) {  /* Only count non-empty lines */
            count++;
        }

        if (!has_newline) break;
    }
    return count;
}

/* Parse response and return true if "Ok" was received (possibly with data) */
static bool parse_response(const char* response, char* response_out, size_t response_size) {
    /* Extract all lines */
    char lines[8][1024];
    int num_lines = extract_lines(response, lines, 8);

    /* Find the "Ok" line and capture all preceding data lines */
    bool found_ok = false;
    size_t out_pos = 0;
    bool found_ok_in_loop = false;

    for (int j = 0; j < num_lines; j++) {
        char* line = lines[j];

        /* Check if this is an async event */
        if (strncmp(line, "event,", 6) == 0) {
            /* Queue the event for later retrieval */
            event_queue_push(line);
            continue;
        }

        /* If this is the "Ok" line, we're done (case-insensitive) */
        /* Convert to uppercase for comparison */
        char line_upper[1024];
        for (int k = 0; line[k]; k++) {
            line_upper[k] = (line[k] >= 'a' && line[k] <= 'z') ? (line[k] - 32) : line[k];
        }
        line_upper[strlen(line)] = '\0';

        if (strstr(line_upper, "OK") != NULL) {
            found_ok = true;
            found_ok_in_loop = true;
            break;  /* "Ok" marks the end of the response */
        }

        /* This is data - add to output */
        size_t line_len = strlen(line);
        if (out_pos + line_len + 2 < response_size) {  /* +2 for newline and null */
            if (out_pos > 0) {
                response_out[out_pos++] = '\n';
            }
            memcpy(response_out + out_pos, line, line_len);
            out_pos += line_len;
        }
    }

    /* If we found "Ok" but the output is empty, add "Ok" to the response
     * so callers can check for it */
    if (found_ok && out_pos == 0 && response_size > 3) {
        strcpy_s(response_out, response_size, "Ok");
        out_pos = 2;
    }

    response_out[out_pos] = '\0';
    return found_ok;
}

bool vt_socket_init(void) {
#ifdef _WIN32
    WSADATA wsa;
    event_queue_init();
    return WSAStartup(MAKEWORD(2,2),&wsa)==0;
#else
    event_queue_init();
    return true;
#endif
}

void vt_socket_cleanup(void) {
#ifdef _WIN32
    WSACleanup();
    if (g_cs_initialized) {
        DeleteCriticalSection(&g_event_cs);
        g_cs_initialized = false;
    }
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
#ifdef _WIN32
    int s = (int)socket(AF_INET, SOCK_STREAM, 0);
    if(s < 0) return false;
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(i->port);
    inet_pton(AF_INET, i->host, &addr.sin_addr);
    if(connect(s, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(s);
        return false;
    }
    i->socket_fd = (SOCKET)s;
    i->is_connected = true;
    return true;
#else
    return false;  /* Not implemented */
#endif
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

    /* Receive response with timeout */
    /* Use a moderate timeout (5 seconds) - shorter for quick commands */
    DWORD timeout = 5000;  /* 5 second timeout */
    setsockopt(i->socket_fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));

    /* Read all available data (may contain events + Ok) */
    ssize_t r = recv(i->socket_fd, response, (int)response_size - 1, 0);
    if(r <= 0) {
        /* Timeout or error */
        response[0] = '\0';
        return false;
    }

    response[r] = '\0';

    /* Parse the response */
    char response_out[2048];
    bool ok = parse_response(response, response_out, sizeof(response_out));

    /* For 'sd' command, the response is the screen content directly (no 'Ok')
     * In this case, use the raw response directly */
    if (strstr(command, "sd") == command) {
        /* Copy raw response to output buffer (already null-terminated at response[r]) */
        strncpy_s(response, response_size, response, response_size - 1);
        return true;  /* sd command always succeeds with screen content */
    }

    /* Copy to output buffer */
    strncpy_s(response, response_size, response_out, response_size - 1);

    return ok;
}

bool vt_socket_send_command_fmt(vt_socket_handle_t handle, const char* command, const char* format, ...) {
    (void)handle;
    (void)command;
    (void)format;
    return true;
}

vt_event_monitor_handle_t vt_socket_start_monitoring(vt_socket_handle_t handle, vt_event_callback_t callback, void* user_data) {
    (void)handle;
    g_event_callback = callback;
    g_event_callback_data = user_data;
    return (vt_event_monitor_handle_t)1;  /* Return non-NULL handle */
}

void vt_socket_stop_monitoring(vt_event_monitor_handle_t monitor) {
    (void)monitor;
    g_event_callback = NULL;
    g_event_callback_data = NULL;
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
    g_event_callback = callback;
    g_event_callback_data = user_data;
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

/* Check if there are pending events in the queue */
bool vt_socket_has_pending_events(void) {
    return event_queue_has_events();
}

/* Pop the next event from the queue */
bool vt_socket_pop_event(char* event, size_t size) {
    return event_queue_pop(event, size);
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
