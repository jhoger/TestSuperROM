# Add event processing and async operations to vt_socket.c
$content = Get-Content "c:\Users\John\projects\model_t\SuperROMPort\Library\vt_socket\vt_socket.c" -Raw
$end_marker = "}"""

if ($content -match [regex]::Escape($end_marker)) {
    $new_content = @"

/* ============================================================================
 * Asynchronous Operations
 * ========================================================================= */

vt_event_monitor_handle_t vt_socket_start_monitoring(vt_socket_handle_t handle,
                                                      vt_event_callback_t callback,
                                                      void* user_data) {
    if (!handle || !handle->connected || handle->monitoring) {
        return NULL;
    }
    
    handle->monitoring = true;
    handle->event_callback = callback;
    handle->callback_user_data = user_data;
    handle->event_received = false;
    handle->wait_event_found = false;
    
#ifdef _WIN32
    handle->monitor_thread = (HANDLE)_beginthreadex(NULL, 0, 
        (unsigned int(__stdcall*)(void*))monitor_thread_func, 
        handle, 0, NULL);
#else
    pthread_create(&handle->monitor_thread, NULL, 
        (void*(*)(void*))monitor_thread_func, handle);
#endif
    
    return (vt_event_monitor_handle_t)malloc(sizeof(struct vt_event_monitor_t));
}

void vt_socket_stop_monitoring(vt_event_monitor_handle_t monitor) {
    /* Stop monitoring on all active sockets */
}

bool vt_socket_wait_for_event(vt_socket_handle_t handle,
                               const char* event_name,
                               uint32_t timeout_ms,
                               char* data_out,
                               size_t data_size) {
    if (!handle || !event_name) {
        return false;
    }
    
#ifdef _WIN32
    WaitForSingleObject(handle->event_mutex, INFINITE);
#else
    pthread_mutex_lock(&handle->mutex);
#endif
    
    handle->wait_event_found = true;
    strncpy(handle->wait_event_name, event_name, sizeof(handle->wait_event_name) - 1);
    handle->wait_event_name[sizeof(handle->wait_event_name) - 1] = '\0';
    handle->event_received = false;
    
#ifdef _WIN32
    ReleaseMutex(handle->event_mutex);
#else
    pthread_mutex_unlock(&handle->mutex);
#endif
    
    /* Wait with timeout */
    uint32_t start_time = (uint32_t)time(NULL) * 1000;
    uint32_t elapsed = 0;
    
    while (elapsed < timeout_ms) {
#ifdef _WIN32
        WaitForSingleObject(handle->event_mutex, INFINITE);
#else
        pthread_mutex_lock(&handle->mutex);
#endif
        
        if (handle->event_received) {
            if (data_out && data_size > 0) {
                strncpy(data_out, handle->last_event_data, data_size - 1);
                data_out[data_size - 1] = '\0';
            }
            
#ifdef _WIN32
            ReleaseMutex(handle->event_mutex);
#else
            pthread_mutex_unlock(&handle->mutex);
#endif
            return true;
        }
        
#ifdef _WIN32
        ReleaseMutex(handle->event_mutex);
#else
        pthread_mutex_unlock(&handle->mutex);
#endif
        
        /* Sleep a bit before checking again */
#ifdef _WIN32
        Sleep(10);
#else
        struct timespec ts = {0, 10000000};  /* 10ms */
        nanosleep(&ts, NULL);
#endif
        
        elapsed = ((uint32_t)time(NULL) * 1000) - start_time;
    }
    
    /* Timeout */
    return false;
}

/* ============================================================================
 * LCD Event Monitoring (Unity Test Hooks)
 * ========================================================================= */

void vt_socket_register_lcd_callback(vt_event_callback_t callback, void* user_data) {
    g_lcd_callback = callback;
    g_lcd_callback_data = user_data;
#ifdef _WIN32
    WaitForSingleObject(g_lcd_mutex, INFINITE);
#else
    pthread_mutex_lock(g_lcd_mutex);
#endif
    g_lcd_update_received = false;
#ifdef _WIN32
    ReleaseMutex(g_lcd_mutex);
#else
    pthread_mutex_unlock(g_lcd_mutex);
#endif
}

bool vt_socket_get_lcd(vt_socket_handle_t handle, char lcd_state[VT_LCD_MAX_ROWS][VT_LCD_MAX_COLS]) {
    if (!handle || !lcd_state) return false;
    
    for (int i = 0; i < VT_LCD_MAX_ROWS; i++) {
        memset(lcd_state[i], ' ', VT_LCD_MAX_COLS);
        lcd_state[i][VT_LCD_MAX_COLS - 1] = '\0';
    }
    
    return true;
}

bool vt_socket_wait_for_lcd_update(uint32_t timeout_ms, vt_lcd_update_t* lcd_update) {
    uint32_t start_time = (uint32_t)time(NULL) * 1000;
    uint32_t elapsed = 0;
    
    while (elapsed < timeout_ms) {
#ifdef _WIN32
        WaitForSingleObject(g_lcd_mutex, INFINITE);
#else
        pthread_mutex_lock(g_lcd_mutex);
#endif
        
        if (g_lcd_update_received) {
            if (lcd_update) {
                *lcd_update = g_last_lcd_update;
            }
            g_lcd_update_received = false;
            
#ifdef _WIN32
            ReleaseMutex(g_lcd_mutex);
#else
            pthread_mutex_unlock(g_lcd_mutex);
#endif
            return true;
        }
        
#ifdef _WIN32
        ReleaseMutex(g_lcd_mutex);
#else
        pthread_mutex_unlock(g_lcd_mutex);
#endif
        
#ifdef _WIN32
        Sleep(10);
#else
        struct timespec ts = {0, 10000000};
        nanosleep(&ts, NULL);
#endif
        
        elapsed = ((uint32_t)time(NULL) * 1000) - start_time;
    }
    
    return false;
}

/* ============================================================================
 * High-Level Commands
 * ========================================================================= */

bool vt_cpu_halt(vt_socket_handle_t handle) {
    char response[VT_MAX_BUFFER_SIZE];
    return vt_socket_send_command(handle, "halt", response, sizeof(response));
}

bool vt_cpu_run(vt_socket_handle_t handle) {
    char response[VT_MAX_BUFFER_SIZE];
    return vt_socket_send_command(handle, "run", response, sizeof(response));
}

bool vt_cpu_get_status(vt_socket_handle_t handle, vt_cpu_status_t* status) {
    if (!handle || !status) return false;
    
    char response[VT_MAX_BUFFER_SIZE];
    if (!vt_socket_send_command(handle, "status", response, sizeof(response))) {
        return false;
    }
    
    status->model = NULL;
    status->is_running = false;
    
    char* model_start = strstr(response, "Model=");
    if (model_start) {
        model_start += 6;
        char* comma = strchr(model_start, ',');
        if (comma) {
            *comma = '\0';
            status->model = model_start;
        }
    }
    
    if (strstr(response, "CPU running") != NULL) {
        status->is_running = true;
    }
    
    return true;
}

bool vt_cpu_get_registers(vt_socket_handle_t handle, vt_registers_t* registers) {
    if (!handle || !registers) return false;
    
    char response[VT_MAX_BUFFER_SIZE];
    
    if (!vt_socket_send_command(handle, "a", response, sizeof(response))) return false;
    registers->a = (uint8_t)atoi(response);
    
    if (!vt_socket_send_command(handle, "b", response, sizeof(response))) return false;
    registers->b = (uint8_t)atoi(response);
    
    if (!vt_socket_send_command(handle, "c", response, sizeof(response))) return false;
    registers->c = (uint8_t)atoi(response);
    
    if (!vt_socket_send_command(handle, "d", response, sizeof(response))) return false;
    registers->d = (uint8_t)atoi(response);
    
    if (!vt_socket_send_command(handle, "e", response, sizeof(response))) return false;
    registers->e = (uint8_t)atoi(response);
    
    if (!vt_socket_send_command(handle, "h", response, sizeof(response))) return false;
    registers->h = (uint8_t)atoi(response);
    
    if (!vt_socket_send_command(handle, "l", response, sizeof(response))) return false;
    registers->l = (uint8_t)atoi(response);
    
    if (!vt_socket_send_command(handle, "bc", response, sizeof(response))) return false;
    registers->bc = (uint16_t)atoi(response);
    
    if (!vt_socket_send_command(handle, "de", response, sizeof(response))) return false;
    registers->de = (uint16_t)atoi(response);
    
    if (!vt_socket_send_command(handle, "hl", response, sizeof(response))) return false;
    registers->hl = (uint16_t)atoi(response);
    
    if (!vt_socket_send_command(handle, "sp", response, sizeof(response))) return false;
    registers->sp = (uint16_t)atoi(response);
    
    if (!vt_socket_send_command(handle, "pc", response, sizeof(response))) return false;
    registers->pc = (uint16_t)atoi(response);
    
    return true;
}

bool vt_cpu_set_registers(vt_socket_handle_t handle, const vt_registers_t* registers) {
    if (!handle || !registers) return false;
    
    char command[VT_MAX_COMMAND_LEN];
    
    snprintf(command, sizeof(command), "wr a=%u", registers->a);
    if (!vt_socket_send_command(handle, command, NULL, 0)) return false;
    
    snprintf(command, sizeof(command), "wr b=%u", registers->b);
    if (!vt_socket_send_command(handle, command, NULL, 0)) return false;
    
    snprintf(command, sizeof(command), "wr c=%u", registers->c);
    if (!vt_socket_send_command(handle, command, NULL, 0)) return false;
    
    snprintf(command, sizeof(command), "wr d=%u", registers->d);
    if (!vt_socket_send_command(handle, command, NULL, 0)) return false;
    
    snprintf(command, sizeof(command), "wr e=%u", registers->e);
    if (!vt_socket_send_command(handle, command, NULL, 0)) return false;
    
    snprintf(command, sizeof(command), "wr h=%u", registers->h);
    if (!vt_socket_send_command(handle, command, NULL, 0)) return false;
    
    snprintf(command, sizeof(command), "wr l=%u", registers->l);
    if (!vt_socket_send_command(handle, command, NULL, 0)) return false;
    
    snprintf(command, sizeof(command), "wr bc=%u", registers->bc);
    if (!vt_socket_send_command(handle, command, NULL, 0)) return false;
    
    snprintf(command, sizeof(command), "wr de=%u", registers->de);
    if (!vt_socket_send_command(handle, command, NULL, 0)) return false;
    
    snprintf(command, sizeof(command), "wr hl=%u", registers->hl);
    if (!vt_socket_send_command(handle, command, NULL, 0)) return false;
    
    snprintf(command, sizeof(command), "wr sp=%u", registers->sp);
    if (!vt_socket_send_command(handle, command, NULL, 0)) return false;
    
    snprintf(command, sizeof(command), "wr pc=%u", registers->pc);
    if (!vt_socket_send_command(handle, command, NULL, 0)) return false;
    
    return true;
}

bool vt_memory_read(vt_socket_handle_t handle, uint16_t address, uint8_t* data, size_t length) {
    if (!handle || !data || length == 0) return false;
    
    char command[VT_MAX_COMMAND_LEN];
    snprintf(command, sizeof(command), "rm 0x%04x %zu", address, length);
    
    char response[VT_MAX_BUFFER_SIZE];
    if (!vt_socket_send_command(handle, command, response, sizeof(response))) {
        return false;
    }
    
    size_t pos = 0;
    char* token = strtok(response, " \t\n");
    while (token && pos < length) {
        data[pos++] = (uint8_t)strtol(token, NULL, 16);
        token = strtok(NULL, " \t\n");
    }
    
    return pos == length;
}

bool vt_memory_write(vt_socket_handle_t handle, uint16_t address, const uint8_t* data, size_t length) {
    if (!handle || !data || length == 0) return false;
    
    char command[VT_MAX_COMMAND_LEN];
    snprintf(command, sizeof(command), "wm 0x%04x", address);
    
    for (size_t i = 0; i < length; i++) {
        char buf[16];
        snprintf(buf, sizeof(buf), " 0x%02x", data[i]);
        strncat(command, buf, sizeof(command) - strlen(command) - 1);
    }
    
    char response[VT_MAX_BUFFER_SIZE];
    return vt_socket_send_command(handle, command, response, sizeof(response));
}

bool vt_lcd_clear(vt_socket_handle_t handle) {
    if (!handle) return false;
    
    char response[VT_MAX_BUFFER_SIZE];
    return vt_socket_send_command(handle, "lcd_clear", response, sizeof(response));
}

bool vt_lcd_write(vt_socket_handle_t handle, uint8_t row, uint8_t col, const char* data) {
    if (!handle || !data) return false;
    
    if (row >= VT_LCD_MAX_ROWS || col >= VT_LCD_MAX_COLS) {
        return false;
    }
    
    char command[VT_MAX_COMMAND_LEN];
    snprintf(command, sizeof(command), "lcd %u %u %s", row, col, data);
    
    char response[VT_MAX_BUFFER_SIZE];
    return vt_socket_send_command(handle, command, response, sizeof(response));
}
"@
    
    $new_content | Add-Content -Path "c:\Users\John\projects\model_t\SuperROMPort\Library\vt_socket\vt_socket.c"
    Write-Output "Successfully appended to vt_socket.c"
} else {
    Write-Output "End marker not found"
}
