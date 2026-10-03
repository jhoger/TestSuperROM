/* VirtualT Process Library - Implementation */

#include "vt_process.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Error message storage - must be defined before functions that use it */
static char last_error[256] = "";

typedef struct vt_process_t {
    VT_PROCESS_PID process_handle;
    VT_PROCESS_PID thread_handle;
    uint16_t port;
    bool is_running;
    bool is_connected;
    vt_socket_handle_t socket_handle;
    vt_process_callback_t callback;
    void* callback_data;
} vt_process_internal_t;

static uint16_t vt_process_find_port(vt_process_handle_t handle);
static bool vt_process_create_process(const vt_process_config_t* config, VT_PROCESS_PID* process_handle, uint16_t port);
static bool vt_process_destroy_process(VT_PROCESS_PID process_handle);

#ifdef _WIN32

static bool vt_process_create_process(const vt_process_config_t* config, VT_PROCESS_PID* process_handle, uint16_t port) {
    char command_line[512];
    char virtualt_path[512];
    char working_dir[512];
    STARTUPINFOA startup_info;
    PROCESS_INFORMATION process_info;
    
    if (!config || !process_handle) return false;
    
    if (config->virtualt_path) {
        strncpy_s(virtualt_path, sizeof(virtualt_path), config->virtualt_path, sizeof(virtualt_path) - 1);
    } else {
        char* env_path = getenv("VIRTUALT_PATH");
        if (env_path) {
            strncpy_s(virtualt_path, sizeof(virtualt_path), env_path, sizeof(virtualt_path) - 1);
        } else {
            strncpy_s(virtualt_path, sizeof(virtualt_path), "virtualt", sizeof(virtualt_path) - 1);
        }
    }
    
    /* If virtualt_path ends with backslash, append virtualt.exe */
    size_t path_len = strlen(virtualt_path);
    if (path_len > 0 && virtualt_path[path_len - 1] == '\\') {
        if (path_len + 12 < sizeof(virtualt_path)) {
            strncpy_s(virtualt_path + path_len, sizeof(virtualt_path) - path_len, "virtualt.exe", 12);
        }
    }
    
    snprintf(command_line, sizeof(command_line), "\"%s\" -p %d", virtualt_path, port);
    
    char* last_slash = strrchr(virtualt_path, '\\');
    if (last_slash) {
        size_t dir_len = last_slash - virtualt_path + 1;
        if (dir_len < sizeof(working_dir)) {
            strncpy_s(working_dir, sizeof(working_dir), virtualt_path, dir_len);
            working_dir[dir_len] = '\0';
        } else {
            /* Directory path too long, use current directory */
            GetCurrentDirectoryA(sizeof(working_dir), working_dir);
        }
    } else {
        GetCurrentDirectoryA(sizeof(working_dir), working_dir);
    }
    
    memset(&startup_info, 0, sizeof(startup_info));
    startup_info.cb = sizeof(startup_info);
    
    if (CreateProcessA(NULL, command_line, NULL, NULL, FALSE, 0, NULL, working_dir, &startup_info, &process_info)) {
        *process_handle = process_info.hProcess;
        CloseHandle(process_info.hThread);
        return true;
    }
    
    /* Debug: Log error information when CreateProcess fails */
    DWORD error = GetLastError();
    snprintf(last_error, sizeof(last_error), "CreateProcess failed with error %lu for command: %s", error, command_line);
    
    return false;
}

static bool vt_process_destroy_process(VT_PROCESS_PID process_handle) {
    if (!process_handle) return true;
    return TerminateProcess(process_handle, 0);
}

#else

static bool vt_process_create_process(const vt_process_config_t* config, VT_PROCESS_PID* process_handle, uint16_t port) {
    (void)config;
    (void)port;
    pid_t pid = fork();
    if (pid < 0) return false;
    if (pid == 0) _exit(127);
    *process_handle = pid;
    return true;
}

static bool vt_process_destroy_process(VT_PROCESS_PID process_handle) {
    if (!process_handle) return true;
    return kill(process_handle, SIGTERM) == 0;
}

#endif

vt_process_handle_t vt_process_create(void) {
    vt_process_internal_t* handle = malloc(sizeof(vt_process_internal_t));
    if (!handle) return NULL;
    memset(handle, 0, sizeof(vt_process_internal_t));
    /* Initialize socket subsystem */
    vt_socket_init();
    return (vt_process_handle_t)handle;
}

void vt_process_destroy(vt_process_handle_t handle) {
    vt_process_internal_t* internal = (vt_process_internal_t*)handle;
    if (!internal) return;
    if (internal->is_running) vt_process_terminate(handle);
    if (internal->socket_handle) vt_socket_destroy(internal->socket_handle);
    free(handle);
    /* Cleanup socket subsystem */
    vt_socket_cleanup();
}

bool vt_process_launch(vt_process_handle_t handle, const vt_process_config_t* config) {
    vt_process_internal_t* internal = (vt_process_internal_t*)handle;
    if (!internal || !config) {
        vt_process_set_error("Invalid arguments to vt_process_launch");
        return false;
    }
    
    if (internal->is_running) vt_process_terminate(handle);
    
    /* Determine port before launching process */
    uint16_t launch_port;
    if (config->port == 0) {
        launch_port = vt_process_find_port(handle);
        if (launch_port == 0) {
            vt_process_set_error("Failed to find available port for VirtualT socket");
            return false;
        }
    } else {
        launch_port = config->port;
    }
    
    if (!vt_process_create_process(config, &internal->process_handle, launch_port)) return false;
    
    internal->port = launch_port;
    internal->is_running = true;
    return true;
}

bool vt_process_wait_for_ready(vt_process_handle_t handle, uint32_t timeout_ms) {
    vt_process_internal_t* internal = (vt_process_internal_t*)handle;
    uint32_t timeout = timeout_ms > 0 ? timeout_ms : 10000;
    uint32_t elapsed = 0, step = 500;
    
    if (!internal || !internal->is_running) return false;
    
    uint16_t port = internal->port;
    if (port == 0) {
        /* No port specified, try to find one */
        port = vt_process_find_port(handle);
        if (port > 0) {
            internal->port = port;
            internal->is_connected = true;
            return true;
        }
        /* Port finding failed */
        vt_process_set_error("Failed to find available port for VirtualT socket");
        return false;
    }
    
    /* Try to connect to the configured port to verify VirtualT is ready */
    while (elapsed < timeout) {
        vt_socket_handle_t sock = vt_socket_create("127.0.0.1", port);
        if (sock) {
            if (vt_socket_connect(sock)) {
                vt_socket_disconnect(sock);
                vt_socket_destroy(sock);
                internal->is_connected = true;
                return true;
            }
            vt_socket_destroy(sock);
        }
        
        /* Debug output */
        printf("[DEBUG] Wait for ready: elapsed=%u, trying again...\n", elapsed);
        
        Sleep(step);
        elapsed += step;
    }
    vt_process_set_error("Wait for ready timed out - VirtualT socket not responding");
    return false;
}

bool vt_process_connect(vt_process_handle_t handle, vt_socket_handle_t socket_handle) {
    vt_process_internal_t* internal = (vt_process_internal_t*)handle;
    if (!internal || !socket_handle) return false;
    internal->socket_handle = socket_handle;
    internal->is_connected = true;
    return true;
}

void vt_process_disconnect(vt_process_handle_t handle) {
    vt_process_internal_t* internal = (vt_process_internal_t*)handle;
    if (!internal) return;
    if (internal->socket_handle) {
        vt_socket_disconnect(internal->socket_handle);
        vt_socket_destroy(internal->socket_handle);
        internal->socket_handle = NULL;
    }
    internal->is_connected = false;
}

bool vt_process_terminate(vt_process_handle_t handle) {
    vt_process_internal_t* internal = (vt_process_internal_t*)handle;
    if (!internal || !internal->is_running) return false;
    
    if (internal->socket_handle) {
        vt_socket_disconnect(internal->socket_handle);
        vt_socket_destroy(internal->socket_handle);
        internal->socket_handle = NULL;
    }
    
    if (internal->process_handle) {
        vt_process_destroy_process(internal->process_handle);
        internal->process_handle = NULL;
    }
    
    internal->is_running = false;
    internal->is_connected = false;
    return true;
}

bool vt_process_get_status(vt_process_handle_t handle, vt_process_status_t* status) {
    vt_process_internal_t* internal = (vt_process_internal_t*)handle;
    if (!internal || !status) return false;
    status->is_running = internal->is_running;
    status->is_connected = internal->is_connected;
    status->port = internal->port;
    status->pid = internal->process_handle;
    return true;
}

uint16_t vt_process_get_port(vt_process_handle_t handle) {
    vt_process_internal_t* internal = (vt_process_internal_t*)handle;
    return internal ? internal->port : 0;
}

bool vt_process_is_running(vt_process_handle_t handle) {
    vt_process_internal_t* internal = (vt_process_internal_t*)handle;
    return internal ? internal->is_running : false;
}

bool vt_process_is_connected(vt_process_handle_t handle) {
    vt_process_internal_t* internal = (vt_process_internal_t*)handle;
    return internal ? internal->is_connected : false;
}

vt_socket_handle_t vt_process_create_socket(vt_process_handle_t handle, const char* host) {
    vt_process_internal_t* internal = (vt_process_internal_t*)handle;
    if (!internal) return NULL;
    
    const char* host_to_use = host ? host : "127.0.0.1";
    vt_socket_handle_t sock = vt_socket_create(host_to_use, internal->port);
    return sock;
}

bool vt_process_set_callback(vt_process_handle_t handle, vt_process_callback_t callback, void* user_data) {
    vt_process_internal_t* internal = (vt_process_internal_t*)handle;
    if (!internal) return false;
    internal->callback = callback;
    internal->callback_data = user_data;
    return true;
}

static uint16_t vt_process_find_port(vt_process_handle_t handle) {
    uint16_t ports[] = {6166, 6167, 6168, 6169, 6170};
    for (int i = 0; i < 5; i++) {
        vt_socket_handle_t sock = vt_socket_create("127.0.0.1", ports[i]);
        if (sock) {
            if (vt_socket_connect(sock)) {
                /* Port is in use, try next */
                vt_socket_disconnect(sock);
                vt_socket_destroy(sock);
                continue;
            }
            /* Port is available - use it */
            vt_socket_destroy(sock);
            return ports[i];
        }
    }
    return 0; /* No available port found */
}

const char* vt_process_get_error(void) {
    return last_error;
}

void vt_process_set_error(const char* msg) {
    strncpy_s(last_error, sizeof(last_error), msg, sizeof(last_error) - 1);
}
