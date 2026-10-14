/*
 * Platform String Compatibility - Stub Implementation (FAILING)
 *
 * These functions are intentionally non-functional to demonstrate
 * that tests fail without proper implementation.
 */

#include "platform_compat.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* Stub implementations - these will cause tests to fail */

int platform_strncpy_s(char *dest, size_t dest_size, const char *src, size_t count) {
    (void)dest;
    (void)dest_size;
    (void)src;
    (void)count;
    /* Stub: always fails */
    return -1;
}

int platform_strcpy_s(char *dest, size_t dest_size, const char *src) {
    (void)dest;
    (void)dest_size;
    (void)src;
    /* Stub: always fails */
    return -1;
}

int platform_strncat_s(char *dest, size_t dest_size, const char *src, size_t count) {
    (void)dest;
    (void)dest_size;
    (void)src;
    (void)count;
    /* Stub: always fails */
    return -1;
}

char* platform_strdup(const char *src) {
    (void)src;
    /* Stub: always returns NULL */
    return NULL;
}
