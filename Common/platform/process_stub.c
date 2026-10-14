/*
 * Platform Process Compatibility - Stub Implementation (FAILING)
 *
 * These functions are intentionally non-functional to demonstrate
 * that tests fail without proper implementation.
 */

#include "platform_compat.h"
#include <stdio.h>

/* Stub implementations - these will cause tests to fail */

int platform_get_last_error(void) {
    /* Stub: returns 0 (no error) - tests will fail */
    return 0;
}

const char* platform_strerror(int err) {
    (void)err;
    /* Stub: returns generic message */
    return "Stub error message";
}

int platform_stricmp(const char *s1, const char *s2) {
    (void)s1;
    (void)s2;
    /* Stub: always returns 0 (equal) - tests will fail */
    return 0;
}

int platform_get_current_dir(char *buf, size_t size) {
    (void)buf;
    (void)size;
    /* Stub: always fails */
    return -1;
}
