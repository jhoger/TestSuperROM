/* VirtualT Process Library - Implementation */

#include "vt_process.h"
#include "platform_compat.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#if !PLATFORM_WINDOWS
#include <unistd.h>
#include <signal.h>
#endif

/* Error message storage - must be defined before functions that use it */
static char last_error[256] = "";

/* Job handle for terminating child processes when parent exits (Windows only) */
#if PLATFORM_WINDOWS
static HANDLE job_handle = NULL;
#endif

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

#if PLATFORM_WINDOWS
static void vt_process_init_job_object(void);
#endif

#if PLATFORM_WINDOWS

static bool vt_process_create_process(const vt_process_config_t* config, VT_PROCESS_PID* process_handle, uint16_t port) {
    char command_line[512];
    char virtualt_path[512];
    char working_dir[512];
    STARTUPINFOA startup_info;
    PROCESS_INFORMATION process_info;

    if (!config || !process_handle) return false;

    if (config->virtualt_path) {
        platform_strncpy_s(virtualt_path, sizeof(virtualt_path), config->virtualt_path, sizeof(virtualt_path) - 1);
    } else {
        char* env_path = getenv("VIRTUALT_PATH");
        if (env_path) {
            platform_strncpy_s(virtualt_path, sizeof(virtualt_path), env_path, sizeof(virtualt_path) - 1);
        } else {
            platform_strncpy_s(virtualt_path, sizeof(virtualt_path), "virtualt", sizeof(virtualt_path) - 1);
        }
    }

    /* Trim trailing whitespace and slashes from path */
    size_t path_len = strlen(virtualt_path);
    while (path_len > 0 && (virtualt_path[path_len - 1] == ' ' ||
                            virtualt_path[path_len - 1] == '\t' ||
                            virtualt_path[path_len - 1] == '\\' ||
                            virtualt_path[path_len - 1] == '/')) {
        virtualt_path[path_len - 1] = '\0';
        path_len = strlen(virtualt_path);
    }

    /* Handle both forward and backward slashes for path separators */
    char last_char = (path_len > 0) ? virtualt_path[path_len - 1] : '\0';

    /* If virtualt_path ends with slash, append platform-specific executable name */
    if (last_char == '\\' || last_char == '/') {
        #if PLATFORM_WINDOWS
        const char* exe_name = "virtualt.exe";
        #else
        const char* exe_name = "virtualt";
        #endif
        if (path_len + strlen(exe_name) < sizeof(virtualt_path)) {
            platform_strncat_s(virtualt_path + path_len, sizeof(virtualt_path) - path_len, exe_name, strlen(exe_name));
            /* Reset path_len after appending */
            path_len = strlen(virtualt_path);
        }
    } else {
        /* Path does not end with slash - check if it already has executable extension */
        #if PLATFORM_WINDOWS
        if (path_len < 4 || platform_stricmp(virtualt_path + path_len - 4, ".exe") != 0) {
            /* No .exe extension - check if there's a dot anywhere (indicating a file with extension) */
            char* last_dot = strrchr(virtualt_path, '.');
            if (last_dot == NULL || (last_dot && last_char == '.')) {
                /* No dot or dot is at the end - treat as directory, append /virtualt.exe */
                if (path_len + 12 < sizeof(virtualt_path)) {
                    platform_strncat_s(virtualt_path, sizeof(virtualt_path), "/virtualt.exe", 13);
                }
            } else {
                /* Has a dot but not .exe - might be another extension, append .exe anyway */
                if (path_len + 4 < sizeof(virtualt_path)) {
                    platform_strncat_s(virtualt_path, sizeof(virtualt_path), ".exe", 4);
                }
            }
        }
        #else
        /* On Linux, could be .so, .out, or no extension - treat as-is */
        /* No extension check needed for simple cases */
        #endif
    }

    snprintf(command_line, sizeof(command_line), "\"%s\" -p %d%s", virtualt_path, port, config->headless ? " -n" : "");

    /* Handle both forward and backward slashes when extracting directory */
    char* last_backslash = strrchr(virtualt_path, '\\');
    char* last_forwardslash = strrchr(virtualt_path, '/');

    /* Use the rightmost slash of either type */
    char* last_slash = (last_backslash > last_forwardslash) ? last_backslash : last_forwardslash;

    if (last_slash) {
        size_t dir_len = last_slash - virtualt_path + 1;
        if (dir_len < sizeof(working_dir)) {
            platform_strncpy_s(working_dir, sizeof(working_dir), virtualt_path, dir_len);
            working_dir[dir_len] = '\0';
        } else {
            /* Directory path too long, use current directory */
            platform_get_current_dir(working_dir, sizeof(working_dir));
        }
    } else {
        /* No slash found, use current directory */
        platform_get_current_dir(working_dir, sizeof(working_dir));
    }

    /* Initialize job object if not already done (for child process cleanup) */
    vt_process_init_job_object();

    memset(&startup_info, 0, sizeof(startup_info));
    startup_info.cb = sizeof(startup_info);

    /* Try with command_line as lpApplicationName to avoid path resolution issues */
    /* CreateProcessA first param is lpApplicationName, second is lpCommandLine */
    /* We need to pass the full command line (with -p port) as the second parameter */
    if (CreateProcessA(NULL, command_line, NULL, NULL, FALSE, 0, NULL, working_dir, &startup_info, &process_info)) {
        *process_handle = process_info.hProcess;
        /* Assign process to job object so it will be terminated when parent exits */
        if (job_handle) {
            AssignProcessToJobObject(job_handle, process_info.hProcess);
        }
        CloseHandle(process_info.hThread);
        return true;
    }

    /* Debug: Log error information when CreateProcess fails */
    DWORD error = GetLastError();
    char error_buf[256];
    snprintf(error_buf, sizeof(error_buf), "CreateProcess failed with error %lu for path: %s, command: %s", error, virtualt_path, command_line);
    vt_process_set_error(error_buf);

    return false;
}

static bool vt_process_destroy_process(VT_PROCESS_PID process_handle) {
    if (!process_handle) return true;

    /* First try to terminate the process */
    bool terminated = TerminateProcess(process_handle, 0);

    /* Wait a moment for cleanup */
    platform_sleep_ms(200);

    /* Wait for process to fully terminate */
    DWORD result = WaitForSingleObject(process_handle, 1000);
    if (result == WAIT_TIMEOUT) {
        /* Process didn't terminate in time, force close */
        CloseHandle(process_handle);
        return false;
    }

    CloseHandle(process_handle);
    return terminated;
}

static void vt_process_init_job_object(void) {
    if (job_handle == NULL) {
        /* Create a job object that kills processes when closed */
        job_handle = CreateJobObject(NULL, NULL);
        if (job_handle) {
            /* Configure job to kill processes when job handle is closed */
            JOBOBJECT_EXTENDED_LIMIT_INFORMATION info = {0};
            info.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
            SetInformationJobObject(job_handle, JobObjectExtendedLimitInformation, &info, sizeof(info));
        }
    }
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
    /* Close job handle to kill any remaining child processes */
#if PLATFORM_WINDOWS
    if (job_handle) {
        CloseHandle(job_handle);
        job_handle = NULL;
    }
#endif
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

    if (!vt_process_create_process(config, &internal->process_handle, launch_port)) {
        return false;  /* Error already set by vt_process_create_process */
    }

    internal->port = launch_port;
    internal->is_running = true;

    /* Give VirtualT a moment to start before waiting for ready */
    platform_sleep_ms(500);

    return true;
}

bool vt_process_wait_for_ready(vt_process_handle_t handle, uint32_t timeout_ms) {
    vt_process_internal_t* internal = (vt_process_internal_t*)handle;
    uint32_t timeout = timeout_ms > 0 ? timeout_ms : 10000;
    uint32_t elapsed = 0, step = 500;

    if (!internal || !internal->is_running) {
        return false;
    }

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

        platform_sleep_ms(step);
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

    /* Small delay to allow process to settle */
#if PLATFORM_WINDOWS
    platform_sleep_ms(100);
#else
    platform_sleep_us(100000);
#endif

#if PLATFORM_WINDOWS
    if (internal->process_handle) {
        vt_process_destroy_process(internal->process_handle);
        internal->process_handle = NULL;
    }
#else
    if (internal->process_handle) {
        vt_process_destroy_process(internal->process_handle);
        internal->process_handle = 0;
    }
#endif

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
    platform_strncpy_s(last_error, sizeof(last_error), msg, sizeof(last_error) - 1);
}
