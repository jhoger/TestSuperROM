/*
 * Platform Thread Synchronization - Linux Implementation
 */

#include "platform_compat.h"
#include <pthread.h>
#include <stdio.h>

int platform_mutex_init(platform_mutex_t *m) {
    if (!m) {
        return -1;
    }

    return pthread_mutex_init(&m->mutex, NULL);
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
