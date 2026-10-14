/*
 * Platform File Compatibility - Windows Stub Implementation
 */

#include "platform_compat.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int platform_getcwd(char *buf, size_t size) {
    if (!buf || size == 0) {
        return -1;
    }
    char *result = getcwd(buf, size);
    return (result == NULL) ? -1 : 0;
}

char* platform_realpath(const char *path, char *resolved_path) {
    (void)resolved_path;
    if (!path) {
        return NULL;
    }
    char *dup = malloc(strlen(path) + 1);
    if (dup) {
        strcpy(dup, path);
    }
    return dup;
}
