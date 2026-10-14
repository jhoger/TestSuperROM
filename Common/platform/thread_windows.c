/*
 * Platform Thread Synchronization - Windows Implementation
 */

#include "platform_compat.h"
#include <stdio.h>

int platform_mutex_init(platform_mutex_t *m) {
    if (!m) {
        return -1;
    }

    InitializeCriticalSection(&m->cs);
    return 0;
}

void platform_mutex_lock(platform_mutex_t *m) {
    if (m) {
        EnterCriticalSection(&m->cs);
    }
}

void platform_mutex_unlock(platform_mutex_t *m) {
    if (m) {
        LeaveCriticalSection(&m->cs);
    }
}

void platform_mutex_destroy(platform_mutex_t *m) {
    if (m) {
        DeleteCriticalSection(&m->cs);
    }
}
