/*
 * Platform Process Compatibility - Linux Implementation
 */

#include "platform_compat.h"
#include <string.h>
#include <errno.h>
#include <strings.h>
#include <unistd.h>
#include <limits.h>

int platform_get_last_error(void) {
    return errno;
}

const char* platform_strerror(int err) {
    return strerror(err);
}

int platform_stricmp(const char *s1, const char *s2) {
    if (!s1 || !s2) {
        return 1;  /* Non-zero for non-equal */
    }
    return strcasecmp(s1, s2);
}

int platform_get_current_dir(char *buf, size_t size) {
    if (!buf || size == 0) {
        return -1;
    }

    char *result = getcwd(buf, size);
    return (result == NULL) ? -1 : 0;
}
