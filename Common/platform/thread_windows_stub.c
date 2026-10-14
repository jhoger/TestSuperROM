/*
 * Platform Thread Synchronization - Windows Stub Implementation
 */

#include "platform_compat.h"
#include <pthread.h>

int platform_mutex_init(platform_mutex_t *m) {
    if (!m) {
        return -1;
    }
    /* Use pthread mutex for stub implementation */
    pthread_mutex_init(&m->mutex, NULL);
    return 0;
}

void platform_mutex_lock(platform_mutex_t *m) {
    if (m) {
        pthread_mutex_lock(&m->mutex);
    }
}

void platform_mutex_unlock(platform_mutex_t *m) {
    if (m) {
        pthread_mutex_unlock(&m->mutex);
    }
}

void platform_mutex_destroy(platform_mutex_t *m) {
    if (m) {
        pthread_mutex_destroy(&m->mutex);
    }
}
