/*
 * Platform String Compatibility - Windows Implementation
 */

#include "platform_compat.h"
#include <string.h>
#include <stdlib.h>

int platform_strncpy_s(char *dest, size_t dest_size, const char *src, size_t count) {
    if (!dest || !src || dest_size == 0) {
        return -1;
    }

    size_t n = (count < dest_size - 1) ? count : dest_size - 1;
    strncpy_s(dest, dest_size, src, n);
    dest[n] = '\0';
    return 0;
}

int platform_strcpy_s(char *dest, size_t dest_size, const char *src) {
    if (!dest || !src || dest_size == 0) {
        return -1;
    }

    size_t len = strlen(src);
    if (len >= dest_size) {
        return -1;
    }

    strcpy_s(dest, dest_size, src);
    return 0;
}

int platform_strncat_s(char *dest, size_t dest_size, const char *src, size_t count) {
    if (!dest || !src || dest_size == 0) {
        return -1;
    }

    size_t dest_len = strlen(dest);
    if (dest_len >= dest_size - 1) {
        return -1;
    }

    size_t remaining = dest_size - 1 - dest_len;
    size_t n = (count < remaining) ? count : remaining;

    strncat_s(dest, dest_size, src, n);
    return 0;
}

char* platform_strdup(const char *src) {
    if (!src) {
        return NULL;
    }

    size_t len = strlen(src) + 1;
    char *dup = _strdup(src);
    return dup;
}
