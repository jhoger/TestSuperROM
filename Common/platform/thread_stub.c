/*
 * Platform Thread Synchronization - Stub Implementation (FAILING)
 *
 * These functions are intentionally non-functional to demonstrate
 * that tests fail without proper implementation.
 */

#include "platform_compat.h"
#include <stdio.h>

/* Stub implementations - these will cause tests to fail */

int platform_mutex_init(platform_mutex_t *m) {
    (void)m;
    /* Stub: always fails */
    return -1;
}

void platform_mutex_lock(platform_mutex_t *m) {
    (void)m;
    /* Stub: does nothing - no synchronization */
    /* Tests will fail race conditions */
}

void platform_mutex_unlock(platform_mutex_t *m) {
    (void)m;
    /* Stub: does nothing - no synchronization */
    /* Tests will fail race conditions */
}

void platform_mutex_destroy(platform_mutex_t *m) {
    (void)m;
    /* Stub: does nothing */
}
