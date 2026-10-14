/*
 * Platform File Compatibility - Windows Implementation
 */

#include "platform_compat.h"
#include <string.h>
#include <stdlib.h>
#include <direct.h>
#include <errno.h>

int platform_getcwd(char *buf, size_t size) {
    if (!buf || size == 0) {
        return -1;
    }

    char *result = _getcwd(buf, (int)size);
    return (result == NULL) ? -1 : 0;
}

char* platform_realpath(const char *path, char *resolved_path) {
    (void)resolved_path;
    if (!path) {
        return NULL;
    }

    char *dup = _strdup(path);
    if (dup) {
        _fullpath(dup, dup, _MAX_PATH);
    }
    return dup;
}
