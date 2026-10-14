/*
 * Platform Process Compatibility - Windows Implementation
 */

#include "platform_compat.h"
#include <string.h>
#include <stdio.h>

int platform_get_last_error(void) {
    return (int)GetLastError();
}

const char* platform_strerror(int err) {
    static char buf[256];

    /* Use FormatMessage to get the system error message */
    DWORD result = FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        (DWORD)err,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        buf,
        sizeof(buf),
        NULL
    );

    if (result == 0) {
        snprintf(buf, sizeof(buf), "Unknown error (code %d)", err);
    }

    return buf;
}

int platform_stricmp(const char *s1, const char *s2) {
    if (!s1 || !s2) {
        return 1;  /* Non-zero for non-equal */
    }
    return _stricmp(s1, s2);
}

int platform_get_current_dir(char *buf, size_t size) {
    if (!buf || size == 0) {
        return -1;
    }

    char *result = _getcwd(buf, (int)size);
    return (result == NULL) ? -1 : 0;
}
