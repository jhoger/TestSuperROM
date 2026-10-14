/*
 * Platform File Compatibility - Linux Implementation
 */

#include "platform_compat.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <stdio.h>

int platform_getcwd(char *buf, size_t size) {
    if (!buf || size == 0) {
        return -1;
    }

    char *result = getcwd(buf, size);
    return (result == NULL) ? -1 : 0;
}

char* platform_realpath(const char *path, char *resolved_path) {
    if (!path) {
        return NULL;
    }

    /* Use realpath to resolve the path */
    char *result = realpath(path, resolved_path);

    if (result == NULL) {
        return NULL;
    }

    /* realpath allocates memory for the result, we need to copy it */
    size_t len = strlen(resolved_path) + 1;
    char *copy = malloc(len);
    if (copy) {
        memcpy(copy, resolved_path, len);
    }

    return copy;
}
