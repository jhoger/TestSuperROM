/*
 * Platform File Compatibility - Stub Implementation (FAILING)
 *
 * These functions are intentionally non-functional to demonstrate
 * that tests fail without proper implementation.
 */

#include "platform_compat.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* Stub implementations - these will cause tests to fail */

int platform_getcwd(char *buf, size_t size) {
    (void)buf;
    (void)size;
    /* Stub: always fails */
    return -1;
}

char* platform_realpath(const char *path, char *resolved_path) {
    (void)path;
    (void)resolved_path;
    /* Stub: always returns NULL */
    return NULL;
}
